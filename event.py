#!/usr/bin/env python
# -*- coding: utf-8 -*-

import types

_subscribers = {}

class func:
    def __init__(self, inst, method):
        self._inst = inst
        self._method = method
    
    def __call__(self, *args, **kwargs):
        # bind method to class instance
        types.MethodType(self._method, self._inst)(*args, **kwargs)

def listener(cls):
   for name, method in cls.__dict__.iteritems():
        if hasattr(method, "subscribe"):
            # methods are still not bound, so store both the instance and the
            # method for late binding
            subscribe(method.subscribe, func(cls, method))
   return cls

def callback(event):
    def decorate(func):
        # func is not yet a class member, so just mark what event it want
        func.subscribe = event
        return func
    return decorate

def trigger(event, payload=None):
    if event not in _subscribers:
        return
    
    for func in _subscribers[event]:
        func(payload)

def subscribe(event, callback):
    if event not in _subscribers:
        _subscribers[event] = []
    
    _subscribers[event].append(callback)
