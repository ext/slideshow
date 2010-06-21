#!/usr/bin/env python
# -*- coding: utf-8 -*-

import multiprocessing, subprocess, sys, threading, traceback
import time, os.path, socket
import dbus, dbus.service
from select import select
from settings import Settings

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

def settings():
	settings = Settings('settings.xml', 'settings.json')
	
	cmd = settings['Files.BinaryPath']
	args = [
		'--uds-log', 'slideshow.sock', 
		'--browser', 'sqlite://%s' % (os.path.abspath('site.db')),
		'--collection-id', str(1),
		'--resolution', settings['Apparence.Resolution']
	]
	env = dict(
		DISPLAY=settings['Apparence.Display'],
		SLIDESHOW_NO_ABORT=''
	)
	for k,v in settings['Env'].items():
		env['SLIDESHOW_' + k] = v
	
	cwd = settings['Path.BasePath']
	
	return (cmd, args, env, cwd)

class _Log:
	def __init__(self, size=5):
		self.size = size
		self.lines = []
	
	def push(self, line):
		self.lines.append(html_escape(line))
		if len(self.lines) > self.size:
			self.lines.pop(0)
	
	def __iter__(self):
		return self.lines.__iter__()

class _Daemon(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
		self._pid = None
		self._ipc = None
		self._instance = None
		self.log = _Log(size=50)
		
		self._state = STOPPED
		self._state_lock = threading.Lock()
		
		self._queue = []
		self._sem = multiprocessing.Semaphore(0) # not using threading.Semaphore since it doesn't support timeouts
		
		self._running = False
	
	def stop(self):
		ipc.Quit()
		while self._instance:
			print 'waiting for instance to terminate'
			time.sleep(1)
		self._running = False
	
	def do_start(self, pid, ipc):
		if not self._state in [STOPPED, CRASHED]:
			raise StateError, 'Cannot start daemon while in state ' + statename(self._state)
	
		try:
			self._state = STARTING
			cmd, args, env, cwd = settings()
			
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
						print line
						self.log.push('stdout: ' + line)
					
					if self._instance.returncode == 0:
						self._state = STOPPED
					else:
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
	
	@dbus.service.signal(dbus_interface='com.slideshow.dbus.Signal', signature='i')
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
	
	sem = threading.Semaphore(0)
	ret = V()
	_daemon.push(func, args, kwargs, sem, ret)
	sem.acquire()
	
	if isinstance(ret.get(), Exception):
		exc_info = ret.get().exc_info
		raise exc_info[0], exc_info[1], exc_info[2]
	
	return ret.get()

def start(pid, ipc):
	return _call(_Daemon.do_start, pid, ipc)

def stop():
	ipc.Quit()
	while _daemon._instance:
			print 'waiting for instance to terminate'
			time.sleep(1)

def state():
	return _daemon.state()

def log():
	return _daemon.log
