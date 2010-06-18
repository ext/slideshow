#!/usr/bin/env python
# -*- coding: utf-8 -*-

import multiprocessing, subprocess, sys, threading, traceback
import time
from settings import Settings

_states = {
	1<<0: 'STOPPED',
	1<<1: 'FINISHED',
	1<<2: 'STOPPING',
	1<<3: 'STARTING',
	1<<4: 'RUNNING',
	1<<5: 'CRASHED'
}

# the reverse of k,v is deliberate
for v,k in _states.items():
	sys.modules[__name__].__dict__[k] = v

def statename(state):
	return _states[state]

class StateError(Exception):
	pass

class _Daemon(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
		self._pid = None
		self._ipc = None
		self._instance = None
		
		self._state = STOPPED
		self._state_lock = threading.Lock()
		
		self._queue = []
		self._sem = multiprocessing.Semaphore(0) # not using threading.Semaphore since it doesn't support timeouts
		
		self._running = False
		
	def stop(self):
		self._running = False
	
	def do_start(self, pid, ipc):
		if not self._state in [STOPPED, FINISHED, CRASHED]:
			raise StateError, 'Cannot start daemon while in state ' + statename(self._state)
	
		self._state = STARTING
		settings = Settings('settings.xml', 'settings.json')
		
		cmd = settings['Files.BinaryPath']
		args = []
		env = dict(
			TERM='xterm'
		)
		print cmd
		self._instance = subprocess.Popen(
			[cmd] + args,
			stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
			cwd=settings['Path.BasePath'], env=None
		)
		
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
				if self._instance.poll():
					self._state = CRASHED
				
				for line in self._instance.stdout:
					print 'stdout:', line, 
				for line in self._instance.stderr:
					print 'stderr:', line, 
			
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
