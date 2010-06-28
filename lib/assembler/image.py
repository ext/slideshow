#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
import subprocess

class ImageAssembler(Assembler):
	def is_editable(self):
		return False
	
	def assemble(self, slide, filename, **kwargs):
		dst = open(slide.src_path(filename.filename), 'wb')
		
		while True:
			chunk = filename.file.read(8192)
			if not chunk:
				break
			dst.write(chunk)
		
		return {'filename': filename.filename}
	
	def rasterize(self, slide, size, params):
		src = params['filename']
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
