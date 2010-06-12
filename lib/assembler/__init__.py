#!/usr/bin/env python
# -*- coding: utf-8 -*-

import subprocess

class Assembler:
	def is_editable(self):
		return False
	
	def rasterize(self, slide, src, size):
		raise NotImplementedError
	
	def default_size(self, slide, src, width=None):
		raise NotImplementedError

class ImageAssembler(Assembler):
	def is_editable(self):
		return False
	
	def rasterize(self, slide, src, size):
		retcode = subprocess.call([
			"convert", slide.src_path(src),
			'-resize', '%dx%d' % size,
			'-background', 'black',
			'-gravity', 'center',
			'-extent', '%dx%d' % size,
			slide.raster_path(size)
		])
		if retcode != 0:
			raise ValueError, 'failed to resample %s' % (self.raster_path(size))
	
	def default_size(self, slide, src, width=None):
		if width:
			raise NotImplementedError
		else:
			return (1024, 768)

class TextAssembler(Assembler):
	def is_editable(self):
		return True

_assemblers = {
	'text': TextAssembler(),
	'image': ImageAssembler()
}

def get(name):
	return _assemblers[name]
