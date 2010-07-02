#!/usr/bin/env python
# -*- coding: utf-8 -*-

class Assembler:
	name = '' # automatically set in factory initialization
	
	def is_editable(self):
		return False
	
	def assemble(self, slide, **kwargs):
		raise NotImplementedError
	
	def rasterize(self, slide, src, size, params):
		raise NotImplementedError
	
	def default_size(self, slide, params, width=None):
		raise NotImplementedError
	
	def raster_is_valid(**kwargs):
		return True

import image
import text

_assemblers = {
	'text': text.TextAssembler,
	'image': image.ImageAssembler
}

# setup reverse names
for k,v in _assemblers.items():
	v.name = k

def get(name):
	return _assemblers[name]()
