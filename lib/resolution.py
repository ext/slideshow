#!/usr/bin/env python
# -*- coding: utf-8 -*-

def validate(func):
    def decorate(input, max):
        ret = func(input, max)
        
        try:
            assert ret.w <= max.w
            assert ret.h <= max.h
            assert abs(input.aspect() - ret.aspect()) < 0.01
        except:
            print 'input: ', input
            print 'size:  ', max
            print 'return:', ret
            raise
        
        return ret
    return decorate

class Resolution:
    def __init__(self, w,h):
        self.w = float(w)
        self.h = float(h)
    
    def aspect(self):
        return self.w / self.h
    
    def __iter__(self):
        return (self.w, self.h).__iter__()
    
    @validate
    def fit(self, max):
        new_size = Resolution(max.w, max.h)
        
        if self.aspect() > max.aspect():
            new_size.h = max.w * (self.h / self.w)
        else:
            new_size.w = max.h * (self.w / self.h)
        
        return new_size
    
    def scale(self, width=None, height=None):
        if width and height:
            raise ValueError, 'both width and height given, can only set one'
        
        if width:
            return Resolution(width, self.h * self.aspect())
        
        if height:
            return Resolution(self.w * self.aspect(), height)
    
    def __str__(self):
        return '%dx%d %.4f' % (self.w, self.h, self.aspect())
