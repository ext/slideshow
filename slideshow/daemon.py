#!/usr/bin/env python
# -*- coding: utf-8 -*-

import multiprocessing, subprocess, sys, threading, traceback
import time, os, re, signal, socket, sqlite3
import cherrypy
from select import select
from settings import Settings
from slideshow.lib import browser
import event
import resource
from cherrypy.process import plugins
import pipes

# used to get a pretty name from signal numbers
_signal_lut = dict((k, v) for v, k in signal.__dict__.iteritems() if v.startswith('SIG') and not v.startswith('SIG_'))

_states = {
    1<<0: 'STOPPED',
    1<<1: 'STOPPING',
    1<<2: 'STARTING',
    1<<3: 'RUNNING',
    1<<4: 'CRASHED'
    }

# the reverse of k,v is deliberate
for v,k in _states.items():
    sys.modules[__name__].__dict__[k] = v

html_escape_table = {
    "&": "&amp;",
    '"': "&quot;",
    "'": "&apos;",
    ">": "&gt;",
    "<": "&lt;",
    " ": "&nbsp;",
    "\t": "&nbsp;" * 4, # 4 is tabwidth
    }

def html_escape(text):
    return "".join(html_escape_table.get(c,c) for c in text)

def statename(state):
    return _states[state]

class StateError(Exception):
    pass

class StartError(Exception):
    def __init__(self, message, stdout):
        Exception.__init__(self, message)
        self.stdout = stdout
        
    def __str__(self):
        stdout = ''.join(self.stdout)
        return self.message + '\n\n' + stdout

def settings(browser, resolution=None, fullscreen=True):
    settings = Settings()
    
    cmd = settings['Files.BinaryPath']
    url = settings['Database.URL']
    args = [
        '--uds-log', 'slideshow.sock',
        '--file-log', 'slideshow.log',
        '--browser', str(browser),
        '--queue-id', str(settings['Runtime.queue']),
        ]
    
    args.append('--resolution')
    if resolution:
        args.append(str(resolution))
    else:
        args.append(str(settings.resolution()))
    
    if isinstance(fullscreen, basestring):
        fullscreen = fullscreen in ['1', 'True', 'true']
    
    if fullscreen:
        args.append('--fullscreen')

    # must be last arg
    args.append(url)
    
    env = dict(
        DISPLAY=settings['Appearance.Display'],
        SLIDESHOW_NO_ABORT='',
        SDL_VIDEO_X11_XRANDR='0',
        HOME=os.environ['HOME'],
        PATH=os.environ['PATH'],
        )
    for k,v in settings['Env'].items():
        env['SLIDESHOW_' + k] = v
    
    cwd = settings['Path.BasePath']
    
    return (cmd, args, env, cwd)

class _Log:
    severity_lut = {
        '!!': 0,
        'WW': 1,
        '  ': 2,
        '--': 3,
        'DD': 4
        }
    severity_revlut = dict([(v,k) for k,v in severity_lut.items()])
    
    def __init__(self, size=5):
        self.size = size
        
    def push(self, line, severity=2):
        try:
            c = cherrypy.thread_data.db

            # default values
            stamp = time.mktime(time.localtime())
            message = line

            # try to guess based on content (this is the output log output format from daemon)
            match = re.match('\((..)\) \[(.*)\] (.*)', line)
            if match:
                severity, stampstr, message = match.groups()
                stamp = time.mktime(time.strptime(stampstr, '%Y-%m-%d %H:%M:%S'))
            
            c.execute("""
                INSERT INTO log (
                    type,
                    severity,
                    stamp,
                    message
                ) VALUES (
                    0,
                    :severity,
                    FROM_UNIXTIME(:stamp),
                    :message
                )
            """, dict(severity=self.severity_lut.get(severity, 2), stamp=int(stamp), message=message))
            cherrypy.thread_data.db.commit()
        except:
            traceback.print_exc()
    
    def __iter__(self):
        c = cherrypy.thread_data.db
        c.commit()
        lines = c.execute("""
            SELECT
                log.severity AS severity,
                user.name AS user,
                UNIX_TIMESTAMP(log.stamp) AS stamp,
                log.message AS message
            FROM
                log,
                user
            WHERE
                log.user_id = user.id
            ORDER BY
                log.stamp DESC,
                log.id DESC
            LIMIT :limit;
        """, dict(limit=self.size)).fetchall()
        lines = list(lines)
        lines.reverse()
        
        def f(severity, user, stamp, message):
            severity_str = self.severity_revlut[severity].replace(' ', '&nbsp;')
            formated_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stamp))
            return severity, '({severity}) {stamp} {user} {message}'.format(severity=severity_str, user=user, stamp=formated_time, message=message)
        
        lines = [f(**x) for x in lines]        
        return lines.__iter__()

class DaemonProcess:
    def __init__(self, logsize=35):
        self._proc = None
        self._returncode = None
        self._log = _Log(size=logsize)
        self._logobj = None
    
    def start(self, resolution, fullscreen):
        if self._proc is not None:
            return
        
        cmd, args, env, cwd = settings(browser.from_settings(Settings()), resolution, fullscreen)
        
        def preexec():
            resource.setrlimit(resource.RLIMIT_CORE, (-1, -1))
        
        try:
            self._returncode = None
            self._proc = subprocess.Popen(
                [cmd] + args,
                stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                cwd=cwd, env=env,
                preexec_fn=preexec
                )
        except Exception, e:
            raise RuntimeError, 'Failed to run `%s`: %s' % (' '.join([pipes.quote(x) for x in [cmd] + args]), e)
        
        self._logobj = self._connect_log(self._proc, cwd)
        time.sleep(1.0) # let daemon settle
    
    def stop(self):
        if self._proc is None:
            return
        
        self._proc.send_signal(signal.SIGINT)
        
        # wait for proper shutdown
        n = 0
        while self._proc:
            self.poll(1.0)
            
            if n > 10: # give up
                print >> sys.stderr, 'Giving up waiting for instance to terminate'
                break
            
            print >> sys.stderr, 'Waiting for instance to terminate'
            time.sleep(1.0)
            n+=1
    
    def reload(self):
        if self._proc is None:
            return
        
        self._proc.send_signal(signal.SIGHUP)
    
    def reset(self):
        self.stop()
        self._returncode = None
    
    def log(self):
        return self._log

    def logmsg(self, line, **kwargs):
        self._log.push(line, **kwargs)
    
    def state(self):
        if self._returncode is not None:
            return CRASHED
        
        if self._proc is None:
            return STOPPED
        
        return RUNNING
    
    def poll(self, timeout):
        proc = self._proc
        
        if proc is None:
            return

        proc.poll()

        # logobj is the log socket between daemon and frontend
        if self._logobj:
            (rd, _, _) = select([self._logobj], [], [], timeout)
            if len(rd) > 0:
                for line in self._logobj.recv(4096).split("\n")[:-1]:
                    self.logmsg(line)
        
        if proc.returncode != None:
            # flush stdout/stderr into the log
            for line in proc.stdout.read().split("\n")[:-1]:
                self.logmsg(line, severity=0)
            
            rc = proc.returncode
            self._proc = None

            if rc != 0:
                self._returncode = rc
                global _signal_lut
                if rc > 0:
                    self._log.push('childprocess exited with abnormal returncode: %d' % rc)
                else:
                    self._log.push('childprocess aborted from signal ' + _signal_lut[-rc])            

    @staticmethod
    def _connect_log(instance, cwd):
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        
        for n in range(0, 20):
            instance.poll()
            
            if instance.returncode != None:
                error = StartError('Instance crashed before log was connected', instance.stdout.readlines())
                raise error
            
            try:
                s.connect(os.path.join(cwd, 'slideshow.sock'))
                return s
            except socket.error:
                time.sleep(0.1)
        
        raise RuntimeError, "Failed to connect log"

class Daemon(plugins.SimplePlugin, threading.Thread):
    instance = None
    
    def __init__(self, bus, browser):
        plugins.SimplePlugin.__init__(self, bus)
        threading.Thread.__init__(self)
    
        self._browser = browser
        self._pid = None
        self._proc = DaemonProcess()
        
        self._queue = []
        self._sem = multiprocessing.Semaphore(0) # not using threading.Semaphore since it doesn't support timeouts
        
        self._running = False
    
    def start(self):
        """
        Start the handler thread.
        """
        
        if not Daemon.instance:
            self.bus.log("Starting daemon handler.")
            threading.Thread.start(self)
            Daemon.instance = self
    
    def graceful(self):
        self.bus.log("Restarting daemon handler.")
        if not Daemon.instance:
            threading.Thread.start(self)
            Daemon.instance = self
    
    def stop(self):
        if not Daemon.instance:
            return
        
        self.bus.log("Stopping daemon handler.")
        
        # stop process
        self.stop_proc()
        
        # stop thread
        self._running = False
        self.join(5.0)
        
        if self.is_alive():
            self.bus.log("Timed out waiting for daemon thread to stop.")
    
    def start_proc(self, resolution, fullscreen):
        return self.call(Daemon._start_proc_int, resolution, fullscreen)
    
    def _start_proc_int(self, resolution, fullscreen):
        """ Should be called from within thread only. """
        return self._proc.start(resolution, fullscreen)
    
    def stop_proc(self):
        return self.call(Daemon._stop_proc_int)
    
    def _stop_proc_int(self):
        """ Should be called from within thread only. """
        return self._proc.stop()
    
    def get_log(self):
        return self._proc.log()
    
    def state(self):
        return self.call(Daemon._state_int)
    
    def _state_int(self):
        return self._proc.state()
    
    def reset(self):
        return self.call(Daemon._reset_int)
    
    def _reset_int(self):
        return self._proc.reset()
    
    def reload(self):
        return self.call(Daemon._reload_int)
    
    def _reload_int(self):
        return self._proc.reload()
    
    def __str__(self):
        return '<daemon id="{id}" />'.format(id=id(self))
    
    def run(self):
        # setup database connection inside daemon thread
        self._browser.connect()

        self._running = True
        while self._running:
            try:
                self._proc.poll(timeout=0.05)
                
                # check if any commands has been issued
                if not self._sem.acquire(timeout=1.0):
                    continue
                
                func, args, kwargs, sem, ret = self._queue.pop()
                try:
                    ret.set(func(self, *args, **kwargs))
                except Exception as e:
                    traceback.print_exc()
                    e.exc_info = sys.exc_info()
                    ret.set(e)
                sem.release()
            except:
                traceback.print_exc()
        
        Daemon.instance = None
        self.bus.log("Daemon handler stopped.")
    
    def call(self, func, *args, **kwargs):
        class V:
            def __init__(self, value=None):
                self.value = value
                
            def set(self, value):
                self.value = value
            
            def get(self):
                return self.value
        
        sem = multiprocessing.Semaphore(0)
        ret = V()
        
        self._queue.append((func, args, kwargs, sem, ret))
        self._sem.release()
        
        if not sem.acquire(timeout=10):
            raise RuntimeError, 'Timeout waiting for call reply: %s.%s %s %s' % (func.im_class.__name__, func.__func__.__name__, args, kwargs)
        
        # pass exceptions to caller
        if isinstance(ret.get(), Exception):
            exc_info = ret.get().exc_info
            raise exc_info[0], exc_info[1], exc_info[2]
        
        return ret.get()

def start(resolution=None, fullscreen=True):
    return Daemon.instance.start_proc(resolution, fullscreen)

def stop():
    return Daemon.instance.stop_proc()

def reset():
    return Daemon.instance.reset()

def reload():
    return Daemon.instance.reload()

def state():
    return Daemon.instance.state()

def log():
    return Daemon.instance.get_log()

@event.listener
class EventListener:
    @event.callback('config.queue_changed')
    def queue_changed(self, id):
        reload()

_listener = EventListener()
