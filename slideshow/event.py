#!/usr/bin/env python
# -*- coding: utf-8 -*-

import types
import cherrypy
import traceback

_subscribers = {}

class func:
    def __init__(self, inst, method, cls):
        self._inst = inst
        self._method = method
        self._cls = cls

    def __call__(self, *args, **kwargs):
        # bind method to class instance
        types.MethodType(self._method, self._inst)(*args, **kwargs)

def listener(cls):
    for name, method in cls.__dict__.iteritems():
        if hasattr(method, "subscribe"):
            # methods are still not bound, so store both the instance and the
            # method for late binding
            subscribe(method.subscribe, func(cls, method, None))
    return cls

def callback(event):
    def decorate(func):
        # func is not yet a class member, so just mark what event it want
        func.subscribe = event
        return func
    return decorate

def trigger(event, *args, **kwargs):
    cherrypy.engine.log('%s triggered' % event)

    if event not in _subscribers:
        return

    for func in _subscribers[event]:
        try:
            func(*args, **kwargs)
        except:
            traceback.print_exc()

def subscribe(event, callback):
    if not isinstance(event, basestring):
        raise ValueError, 'event name must be string (got %s)' % event.__class__.__name__

    if event not in _subscribers:
        _subscribers[event] = []

    _subscribers[event].append(callback)
