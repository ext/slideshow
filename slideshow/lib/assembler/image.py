#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
from slideshow.lib.resolution import Resolution
import subprocess, pipes
import PythonMagick

class ImageAssembler(Assembler):
	def is_editable(self):
		return False
	
	def assemble(self, slide, filename, **kwargs):
		if filename.filename == '':
			raise RuntimeError, 'No file selected'

		dst = open(slide.src_path(filename.filename), 'wb')
		
		while True:
			chunk = filename.file.read(8192)
			if not chunk:
				break
			dst.write(chunk)
		
		return {'filename': filename.filename}
	
	def rasterize(self, slide, size, params):
		if isinstance(size, tuple):
			raise ValueError, 'omg'
		
		src = params['filename']
		args = [
			"convert", slide.src_path(src),
			'-resize', str(size),
			'-background', 'black',
			'-gravity', 'center',
			'-extent', str(size),
			slide.raster_path(size)
		]
		
		try:
			retcode = subprocess.call(args)
		except OSError, e:
			raise RuntimeError, 'Failed to run `%s`: %s' % (' '.join([pipes.quote(x) for x in args]), e) 
		
		if retcode != 0:
			raise ValueError, 'failed to resample %s' % (slide.raster_path(size))
	
	def default_size(self, slide, params, width=None):
		src = params['filename']
		img = PythonMagick.Image(str(slide.src_path(src)))
		geom = img.size()
		size = Resolution(geom.width(), geom.height())
		
		if width:
			return size.scale(width=width)
		else:
			return size
	
	def title(self):
		return 'Image'
