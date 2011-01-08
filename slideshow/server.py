import os,sys
import socket
import traceback
import threading
import json
import sqlite3
import subprocess
import select
import collections

db_file = 'slideshow.db'

def is_callable(x):
    return isinstance(x, collections.Callable)

class ReplyObject:
    def __init__(self):
        self._flag = threading.Event()
        self._value = None

    def set(self, v):
        self._value = v
        self._flag.set()

    def get(self):
        self._flag.wait()
        return self._value

class Instance(threading.Thread):
    def __init__(self, path='./a.out', cwd='.'):
        threading.Thread.__init__(self)
        self._path = path
        self._cwd = cwd

        self._rc = None
        self._rc_lock = threading.Lock()

        self._queue = []
        self._queue_lock = threading.Lock()
        self._queue_sema = threading.Semaphore(0)

    def run(self):
        self._set_rc(None)
        kwargs={}
        while True:
            try:
                kwargs = self.do_run(**kwargs)
            except Exception, e:
                traceback.print_exc()

    def do_run(self, **kwargs):
        msg, reply = self._receive()
        cmd, args = msg[0], msg[1:]
        name = '_cmd_%s' % (cmd)
        func = getattr(self, name, None)
        
        if is_callable(func):
            try:
                print 'args', args
                print 'kwargs', kwargs
                kwargs, rc = func(*args, **kwargs)
                reply.set(rc)
                return kwargs
            except Exception, e:
                    exc_type, exc_value, exc_tb = sys.exc_info()
                    data = traceback.format_exception(exc_type, exc_value, exc_tb)
                    reply.set((-1, data))                
        else:
            raise AttributeError, 'no function matching %s (%s)' % (cmd, name)

        return kwargs

    def _receive(self):
        self._queue_sema.acquire()
        self._queue_lock.acquire()
        msg = self._queue.pop()
        self._queue_lock.release()
        return msg
        
    def _send(self, msg):
        self._queue_lock.acquire()
        self._queue.append(msg)
        self._queue_lock.release()
        self._queue_sema.release()

    def rpc(self, msg):
        reply = ReplyObject()
        self._send((msg, reply))
        return reply.get()

    def _cmd_start(self, **kwargs):
        return kwargs
        
        #proc = subprocess.Popen(['slideshow'], executable=self._path, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=self._cwd)

        #poll = select.poll()
        ##poll.register(proc.stdin, select.POLLOUT)
        #poll.register(proc.stdout, select.POLLIN)
        #poll.register(proc.stderr, select.POLLIN)

        #while proc.returncode == None:
        #    print 'waiting for event'
        #    for (fd, event) in poll.poll():
        #        if fd == proc.stdout.fileno():
        #            print proc.stdout.readline(),
        #        elif fd == proc.stderr.fileno():
        #            print proc.stderr.readline(),
        #        else:
        #            print 'unknown fd'
        #        
        #    proc.poll()
        #self._set_rc(proc.returncode)

    def rc(self):
        self._rc_lock.acquire()
        rc = self._rc
        self._rc_lock.release()
        return rc

    def _set_rc(self, rc):
        self._rc_lock.acquire()
        self._rc = rc
        self._rc_lock.release()

instance = Instance()
instance.start()

class Worker:
    def __init__(self):
        pass

    # call from thread, not parent
    def init(self):
        self._db = sqlite3.connect(db_file)
        self._c = self._db.cursor()

    def do_get_queues(self):
        self._c.execute('SELECT name FROM queue')
        return [x[0] for x in  self._c.fetchall()]

    def do_get_slides(self, queue):
        return [queue]
    
    def do_get_status(self):
        return instance.rc()

    def do_start(self):
        return instance.rpc(('start', 234, 'boof'))

    def do_stop(self):
        return instance.rpc(444)

class IPCWorker(Worker):
    def __init__(self, socket):
        Worker.__init__(self)
        self._socket = socket

    def __call__(self):
        try:
            self.init()
            while True:
                line = self._socket.recv(1024)
                if len(line) == 0:
                    continue

                try:
                    reply = self.handle_line(line)
                    self._socket.send(json.dumps(reply))
                except Exception, e:
                    exc_type, exc_value, exc_tb = sys.exc_info()
                    data = traceback.format_exception(exc_type, exc_value, exc_tb)
                    self._socket.send(json.dumps((-1, data)))
        except socket.error, e:
            pass
        except Exception, e:
            traceback.print_exc()

    def handle_line(self, line):
        print 'line:', line
        tokens = json.loads(line)
        print 'tokens:', tokens
        name = 'do_' + str(tokens[0])
        fun = None
        try:
            func = getattr(self, name)
        except AttributeError:
            raise NotImplementedError, '%s is not implemented' % name

        return func(*tokens[1:])

class IPC(threading.Thread):
    def __init__(self, file='.ipc'):
        threading.Thread.__init__(self, name='IPC-Thread')
        self._file = file

    def run(self):
        try:
            os.remove(self._file)
        except:
            pass

        try:
            s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            s.bind('.ipc')
            s.listen(1)
            
            while True:
                conn, _ = s.accept()
                worker = threading.Thread(target=IPCWorker(conn))
                worker.start()

        except Exception, e:
            print 'here'
            traceback.print_tb(None, file=sys.stdout)

if '--daemon' in sys.argv:
    print 'starting as daemon'
    pid = os.fork()
    if pid:
        sys.exit(0)
    else:
        ipc = IPC()
        ipc.start()
else:
    print 'starting in foreground'
    ipc = IPC()
    ipc.start()
