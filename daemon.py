#!/usr/bin/env python
# -*- coding: utf-8 -*-

import multiprocessing, subprocess, sys, threading, traceback
import time, os.path, socket
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

def statename(state):
	return _states[state]

class StateError(Exception):
	pass

class _Log:
	def __init__(self, size=5):
		self.size = size
		self.lines = []
	
	def push(self, line):
		self.lines.append(line)
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
	
	def __del__(self):
		print '__del__'
		self._instance = None
		
	def stop(self):
		self._running = False
	
	def do_start(self, pid, ipc):
		if not self._state in [STOPPED, CRASHED]:
			raise StateError, 'Cannot start daemon while in state ' + statename(self._state)
	
		self._state = STARTING
		settings = Settings('settings.xml', 'settings.json')
		
		cmd = settings['Files.BinaryPath']
		args = [
			'--uds-log', 'slideshow.sock', 
			'--browser', 'sqlite://%s' % (os.path.abspath('site.db'))
		]
		env = dict(
			#TERM='xterm'
			DISPLAY=settings['Apparence.Display']
		)
		env.update(settings['Env'])
		print cmd, args, env
		instance = subprocess.Popen(
			[cmd] + args,
			stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
			cwd=settings['Path.BasePath'], env=env
		)
		
		self._logobj = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
		
		n = 0
		while True:
			n += 1
			
			if n == 20:
				raise RuntimeError, "Failed to connect log"
			
			try:
				self._logobj.connect(os.path.join(settings['Path.BasePath'], 'slideshow.sock'))
				break
			except:
				time.sleep(0.1)
		
		self._instance = instance
		
		self._state = RUNNING
	
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
					for line in self._logobj.recv(4096).split("\n"):
						self.log.push(line)
				
				if self._instance.returncode != None:
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

_daemon = _Daemon()

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
	
	print 'return', ret.get()
	return ret.get()

def start(pid, ipc):
	return _call(_Daemon.do_start, pid, ipc)

def state():
	return _daemon.state()

def log():
	return _daemon.log