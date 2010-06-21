#!/usr/bin/env python
# -*- coding: utf-8 -*-

import subprocess
import array, cairo, json

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
		dst = open(slide.src_path(filename.filename), 'wb')
		
		while True:
			chunk = filename.file.read(8192)
			if not chunk:
				break
			dst.write(chunk)
		
		return {'filename': filename.filename}
	
	def rasterize(self, slide, filename, size):
		src = filename
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

class Template:
	def __init__(self, filename):
		pass
	
	def rasterize(self, dst, size, params):
		w,h = size
		data = array.array('c', chr(0) * w * h * 4)
		surface = cairo.ImageSurface.create_for_data(data, cairo.FORMAT_ARGB32, w, h, w*4)
		cr = cairo.Context(surface)
		
		cr.save()
		cr.set_source_rgba(1,0,1,1)
		cr.set_operator(cairo.OPERATOR_SOURCE)
		cr.paint()
		cr.restore()
		
		x=25.6
		y=128.0
		x1=102.4
		y1=230.4
		x2=153.6
		y2=25.6
		x3=230.4
		y3=128.0
		
		cr.move_to (x, y);
		cr.curve_to (x1, y1, x2, y2, x3, y3);
		
		cr.set_line_width (10.0);
		cr.stroke ();
		
		cr.set_source_rgba (1, 0.2, 0.2, 0.6);
		cr.set_line_width (6.0);
		cr.move_to (x,y);   cr.line_to (x1,y1);
		cr.move_to (x2,y2); cr.line_to (x3,y3);
		cr.stroke ();
		
		surface.write_to_png(dst)

class TextAssembler(Assembler):
	def is_editable(self):
		return True
	
	def assemble(self, slide, **kwargs):
		return kwargs
	
	def rasterize(self, slide, size, **kwargs):
		template = Template('nitroxy.xml')
		template.rasterize(dst=slide.raster_path(size), size=size, params=kwargs)

_assemblers = {
	'text': TextAssembler(),
	'image': ImageAssembler()
}

# setup reverse names
for k,v in _assemblers.items():
	v.name = k

def get(name):
	return _assemblers[name]
