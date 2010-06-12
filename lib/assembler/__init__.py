#!/usr/bin/env python
# -*- coding: utf-8 -*-

import subprocess

class Assembler:
	name = '' # automatically set in factory initialization
	
	def is_editable(self):
		return False
	
	def assemble(self, slide, **kwargs):
		raise NotImplementedError
	
	def rasterize(self, slide, src, size):
		raise NotImplementedError
	
	def default_size(self, slide, src, width=None):
		raise NotImplementedError

class ImageAssembler(Assembler):
	def is_editable(self):
		return False
	
	def assemble(self, slide, filename):
		print dir(filename)
		print filename.filename
		dst = open(slide.src_path(filename.filename), 'wb')
		
		while True:
			chunk = filename.file.read(8192)
			if not chunk:
				break
			dst.write(chunk)
		
		return filename.filename

	
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

# setup reverse names
for k,v in _assemblers.items():
	v.name = k

def get(name):
	return _assemblers[name]
