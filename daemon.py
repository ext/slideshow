#!/usr/bin/env python
# -*- coding: utf-8 -*-

import multiprocessing, subprocess, sys, threading, traceback
import time, os.path, re, signal, socket, sqlite3
import dbus, dbus.service
import cherrypy
from select import select
from settings import Settings
import event

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

def settings(resolution=None, fullscreen=True):
	settings = Settings()
	
	cmd = settings['Files.BinaryPath']
	args = [
		'--uds-log', 'slideshow.sock', 
		'--browser', 'sqlite://%s' % (os.path.abspath('site.db')),
		'--collection-id', str(settings['Runtime.queue']),
	]
	
	args.append('--resolution')
	if resolution:
		print 'using custom resolution'
		args.append(str(resolution))
	else:
		args.append(str(settings.resolution()))
	
	if isinstance(fullscreen, basestring):
		fullscreen = fullscreen in ['1', 'True', 'true']
	
	if fullscreen:
		args.append('--fullscreen')
	
	env = dict(
		DISPLAY=settings['Apparence.Display'],
		SLIDESHOW_NO_ABORT='',
		SDL_VIDEO_X11_XRANDR='0'
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
		
	def push(self, line):
		try:
			c = cherrypy.thread_data.db.cursor()
			match = re.match('\((..)\) \[(.*)\] (.*)', line)
			
			severity = 2
			stamp = time.mktime(time.localtime())
			message = line
			
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
					:stamp,
					:message
				)
			""", dict(severity=self.severity_lut.get(severity, 2), stamp=stamp, message=html_escape(message)))
			cherrypy.thread_data.db.commit()
		except:
			traceback.print_exc()
	
	def __iter__(self):
		c = cherrypy.thread_data.db.cursor()
		lines = c.execute("""
			SELECT
				log.severity as severity,
				user.name as user,
				log.stamp as stamp,
				log.message as message
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
		lines.reverse()
		
		def f(severity, user, stamp, message):
			severity_str = self.severity_revlut[severity].replace(' ', '&nbsp;')
			formated_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stamp))
			return '({severity}) {stamp} {user} {message}'.format(severity=severity_str, user=user, stamp=formated_time, message=message)
		
		lines = [f(**x) for x in lines]
		
		return lines.__iter__()

class _Daemon(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
	
		self._pid = None
		self._ipc = None
		self._instance = None
		self.log = _Log(size=35)
		
		self._state = STOPPED
		self._state_lock = threading.Lock()
		
		self._queue = []
		self._sem = multiprocessing.Semaphore(0) # not using threading.Semaphore since it doesn't support timeouts
		
		self._running = False
	
	def stop(self):
		ipc.Quit()
		
		# wait for proper shutdown
		while self._instance:
			try:
				os.kill(self._instance.pid, 0) # try if process accepts signals
			except OSError:
				# does not accept signals, process is fubar or lost, cannot continue
				print 'childprocess is fubar or lost, aborting'
				self._instance = None
				break
			
			print 'waiting for instance to terminate'
			time.sleep(1)
		
		self._running = False
	
	def reset(self):
		ipc.Quit()
		self._running = False
		self._state = STOPPED
	
	def do_start(self, resolution, fullscreen):
		if not self._state in [STOPPED, CRASHED]:
			raise StateError, 'Cannot start daemon while in state ' + statename(self._state)
	
		try:
			self._state = STARTING
			cmd, args, env, cwd = settings(resolution, fullscreen)
			print 'cmd:', cmd
			print 'args:', args
			print 'env:', env
			print 'cwd:', cwd
			
			instance = subprocess.Popen(
				[cmd] + args,
				stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
				cwd=cwd, env=env
			)
			
			self._logobj = self._connect_log(instance, cwd)
			self._instance = instance
			
			self._state = RUNNING
		except:
			self._state = CRASHED
			raise
	
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
	
	def state(self):
		self._state_lock.acquire()
		tmp = self._state
		self._state_lock.release()
		return tmp
	
	def __str__(self):
		return '<daemon id="{id}" />'.format(id=id(self))

	def run(self):
		cherrypy.thread_data.db = sqlite3.connect('site.db')
		cherrypy.thread_data.db.row_factory = sqlite3.Row
		cherrypy.thread_data.db.cursor().execute('PRAGMA foreign_keys = ON')
		
		self._running = True
		while self._running:
			if self._instance:
				self._instance.poll()
				
				(rd, _, _) = select([self._logobj], [], [], 0)
				if len(rd) > 0:
					for line in self._logobj.recv(4096).split("\n")[:-1]:
						self.log.push(line)
				
				if self._instance.returncode != None:
					# poll std{out,err}
					for line in self._instance.stdout:
						self.log.push(line)
					
					rc = self._instance.returncode
					if rc == 0:
						self._state = STOPPED
					else:
						global _signal_lut
						if rc > 0:
							self.log.push('childprocess exited with abnormal returncode:' + rc)
						else:
							self.log.push('childprocess aborted from signal ' + _signal_lut[-rc])
						self._state = CRASHED
					
					self._instance = None
			
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
	
	def push(self, func, args, kwargs, sem, ret):
		self._queue.append((func, args, kwargs, sem, ret))
		self._sem.release()

class _DBus(dbus.service.Object):
	def __init__(self):
		dbus.service.Object.__init__(self, dbus.SystemBus(), '/com/slideshow/dbus/ping')

	@dbus.service.signal(dbus_interface='com.slideshow.dbus.Signal', signature='')
	def Quit(self):
		pass
	
	@dbus.service.signal(dbus_interface='com.slideshow.dbus.Signal', signature='')
	def Ping(self):
		pass
	
	@dbus.service.signal(dbus_interface='com.slideshow.dbus.Signal', signature='')
	def Reload(self):
		pass
	
	@dbus.service.signal(dbus_interface='com.slideshow.dbus.Signal', signature='')
	def Debug_DumpQueue(self):
		pass
	
	@dbus.service.signal(dbus_interface='com.slideshow.dbus.Signal', signature='u')
	def ChangeQueue(self, id):
		pass

from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)

_daemon = _Daemon()
ipc = _DBus()

def subscribe(engine):
	engine.subscribe('start', _daemon.start)
	engine.subscribe('stop', _daemon.stop)

def _call(func, *args, **kwargs):
	class V:
		def __init__(self, value=None):
			self.value = value
			
		def set(self, value):
			self.value = value
		
		def get(self):
			return self.value
	
	sem = multiprocessing.Semaphore(0)
	ret = V()
	_daemon.push(func, args, kwargs, sem, ret)
	if not sem.acquire(timeout=10):
		raise RuntimeError, 'Timeout waiting for call reply'
	
	if isinstance(ret.get(), Exception):
		exc_info = ret.get().exc_info
		raise exc_info[0], exc_info[1], exc_info[2]
	
	return ret.get()

def start(resolution=None, fullscreen=True):
	print 'daemon.start:', resolution
	return _call(_Daemon.do_start, resolution, fullscreen)

def stop():
	ipc.Quit()
	while _daemon._instance:
			print 'waiting for instance to terminate'
			time.sleep(1)

def reset():
	_daemon.reset()

def state():
	return _daemon.state()

def log():
	return _daemon.log

@event.listener
class EventListener:
	@event.callback('config.queue_changed')
	def queue_changed(self, id):
		ipc.ChangeQueue(id)

_listener = EventListener()
