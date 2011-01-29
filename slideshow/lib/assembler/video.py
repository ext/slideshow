#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
import os, os.path, subprocess
from slideshow.settings import Settings

class VideoAssembler(Assembler):
	def is_editable(self):
		return False
	
	def assemble(self, slide, filename, **kwargs):
		return {'filename': filename}
	
	def rasterize(self, slide, size, params):
		filename = params['filename']
		src, ext = os.path.splitext(filename)
		settings = Settings()
		base = settings['Path.BasePath']
		video = settings['Path.Video']
		
		args = [
			'ffmpeg',
			'-i', os.path.join(base, video, filename),
			'-ss', '5', # just 5s into video
			'-s', str(size),
			'-an', # skip audio
			'-f', 'image2',
			'-vframes', '1',
			slide.raster_path(size)
		]
		try:
			retcode = subprocess.call(args)
		except OSError, e:
			raise RuntimeError, 'Failed to run `%s`: %s' % (' '.join([pipes.quote(x) for x in args]), e) 
		
		# try to create a symlink to the actual file, expected to fail since any
		# previous rasterisations would had created this symlink already.
		try:
			os.symlink(os.path.join(base, video, filename), slide.src_path('video'))
		except OSError:
			pass
	
	def default_size(self, slide, params, width=None):
		pass
	
	def title(self):
		return 'Video'
	
	def raster_extension(self):
		return '.png'
	
	def render(self, content, context):
		content = content.copy()
		
		settings = Settings()
		base = settings['Path.BasePath']
		video = settings['Path.Video']
		videopath = os.path.join(base, video)
		
		content['files'] = os.listdir(videopath)
		return Assembler.render(self, content, context)
