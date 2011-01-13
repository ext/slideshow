#!/usr/bin/env python
# -*- coding: utf-8 -*-

class Resolution:
    def __init__(self, w,h):
        self.w = float(w)
        self.h = float(h)
    
    def aspect(self):
        return self.w / self.h
    
    def __iter__(self):
        return (self.w, self.h).__iter__()
    
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
            width = float(width)
            return Resolution(width, width / self.aspect())
        
        if height:
            height = float(height)
            return Resolution(height * self.aspect(), height)
    
    def __str__(self):
        return '%dx%d' % (int(self.w), int(self.h))
