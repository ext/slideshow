#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
import os, os.path, subprocess
from settings import Settings

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
		temp = settings['Path.Temp']
		
		cache = os.path.join(base, temp, src + '.gif')
		# convert to gif (cached since this is kept the same size for all rasters)
		if not os.path.exists(cache):
			retcode = subprocess.call([
				'ffmpeg',
				'-i', os.path.join(base, video, filename),
				'-pix_fmt', 'rgb24', # force rgb24
				'-loop_output', '0', # loop forever
				'-r', '1/25', # 1/25 fps
				'-ss', '5', # just 5s into video
				'-f', 'gif', # format
				'-an', # skip audio
				'-y', # overwrite dst
				
			])
			if retcode != 0:
				raise ValueError, 'failed to resample %s' % (slide.raster_path(size))
		
		# asjust speed of gif
		retcode = subprocess.call([
			"convert",
			'-delay', '100',
			cache,
			'-resize', str(size),
			'-background', 'black',
			'-gravity', 'center',
			'-extent', str(size),
			slide.raster_path(size)
		])
		if retcode != 0:
			raise ValueError, 'failed to resample %s' % (slide.raster_path(size))
		
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
		return '.gif'
	
	def render(self, content):
		content = content.copy()
		
		settings = Settings()
		base = settings['Path.BasePath']
		video = settings['Path.Video']
		videopath = os.path.join(base, video)
		
		content['files'] = os.listdir(videopath)
		return Assembler.render(self, content)
