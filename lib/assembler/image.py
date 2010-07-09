#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
from lib.resolution import Resolution
import subprocess
import PythonMagick

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
		if isinstance(size, tuple):
			raise ValueError, 'omg'
		
		src = params['filename']
		retcode = subprocess.call([
			"convert", slide.src_path(src),
			'-resize', str(size),
			'-background', 'black',
			'-gravity', 'center',
			'-extent', str(size),
			slide.raster_path(size)
		])
		if retcode != 0:
			raise ValueError, 'failed to resample %s' % (slide.raster_path(size))
	
	def default_size(self, slide, params, width=None):
		print 'default size'
		
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
	
	def render(self, content):
		return """<fieldset>
			<p>Using this option you can upload an existing image. The image will be scaled to fit the screen but will keep the aspect ratio.</p>
			<input type="file" id="filename" name="filename" accept="image/*" />
			<input type="submit" value="Upload" />
		</fieldset>"""