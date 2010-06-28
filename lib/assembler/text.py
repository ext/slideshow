#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
import array, cairo, pango, pangocairo, json, re
import xml
from xml.dom import minidom
from htmlcolor import Parser as ColorParser

def decode_position(str, size):
	"""
	Decode a position string to a tuple with the element sizes
	Accepts either absolute positions or as a percentage of the full size.
	Negative positions are subtracted from the size.
	
	'-50 50%' -> (750, 300) (given that size is (800,600))
	"""
	
	if str == '':
		return None
	
	# parse an element
	# x is element
	# s is the total dimension
	def f(x,s):
		if x[-1] == '%':
			v = float(x[:-1]) / 100.0 * s
		else:
			v = float(x)
		
		if v < 0.0:
			v = s + v
		
		return v
	
	# split into pieces 'x y' -> [x, y]
	p = str.split(' ')
	
	# parse each element
	p = [f(x,s) for x,s in zip(p, size)]
	
	# create tuple
	return tuple(p)

color_parser = ColorParser(components=4)
decode_color = color_parser.parse

class Template:
	def __init__(self, filename):
		self._filename = filename
	
	def rasterize(self, dst, size, params):
		doc = minidom.parse(self._filename)
		template = doc.getElementsByTagName('template')[0]
		
		resolution = list(params['resolution'])
		aspect = float(resolution[0]) / resolution[1]
		
		# fit resolution in size by scaling
		w = int(size[1] * aspect)
		h = size[1]
		realsize = (w,h)
		
		# scale constant
		scale = float(w) / resolution[0]
		
		data = array.array('c', chr(0) * w * h * 4)
		surface = cairo.ImageSurface.create_for_data(data, cairo.FORMAT_ARGB32, w, h, w*4)
		cr = cairo.Context(surface)
		
		cr.save()
		cr.set_source_rgba(0,0,0,1)
		cr.set_operator(cairo.OPERATOR_SOURCE)
		cr.paint()
		cr.restore()
		
		for item in template.childNodes:
			if not isinstance(item, minidom.Element):
				continue
			
			cr.save()
			try:
				if item.tagName == 'text':
					self._text(size, realsize, scale, cr, item, params[item.getAttribute('name')])
				elif item.tagName == 'textarea':
					self._textarea(size, realsize, scale, cr, item, params[item.getAttribute('name')])
			finally:
				cr.restore()
			
		surface.write_to_png(dst)
	
	@staticmethod
	def _text(size, realsize, scale, cr, item, text):
		font = item.getAttribute('font') or 'Sans'
		fontsize = float(item.getAttribute('size') or '36.0')
		r,g,b,a = decode_color(item.getAttribute('color')) or (0,0,0,1)
		x,y = decode_position(item.getAttribute('position'), realsize) or (0,0)
		alignment = item.getAttribute('align')
		
		fontsize *= scale
		
		cr.select_font_face (font, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)
		cr.set_font_size(fontsize)
		cr.set_source_rgba(r,g,b,a)
		
		extents = cr.text_extents(text)
		
		offset = 0
		if alignment == "center":
			x -= extents[4] / 2
		elif alignment == "right":
			x -= extents[4]
		
		cr.move_to(x, y)
		cr.show_text(text)
	
	@staticmethod
	def _textarea(size, realsize, scale, cr, item, text):
		#realsize = (size[0] * scale, size[1] * scale)
		font = item.getAttribute('font') or 'Sans'
		fontsize = float(item.getAttribute('size') or '36.0')
		r,g,b,a = decode_color(item.getAttribute('color')) or (0,0,0,1)
		x,y = decode_position(item.getAttribute('position'), realsize) or (0,0)
		w,h = decode_position(item.getAttribute('boxsize'), realsize) or size
		alignment = item.getAttribute('align')
		
		fontsize *= scale
		
		cr.set_source_rgba(r,g,b,a)
		cr.move_to(x-w/2.0, y-h/2.0)
		
		ctx = pangocairo.CairoContext(cr)
		layout = ctx.create_layout()
		layout.set_font_description(pango.FontDescription('%s %f' % (font, fontsize)))
		layout.set_width(int(w * pango.SCALE))
		
		if alignment == 'center':
			layout.set_alignment(pango.ALIGN_CENTER)
		elif alignment == 'right':
			layout.set_alignment(pango.ALIGN_RIGHT)
		elif alignment == 'justify':
			layout.set_justify(True)
		
		layout.set_markup(text);
		ctx.show_layout(layout)

class TextAssembler(Assembler):
	def is_editable(self):
		return True
	
	def assemble(self, slide, **kwargs):
		return kwargs
	
	def rasterize(self, size, slide=None, file=None, **kwargs):
		dst = slide and slide.raster_path(size) or file
		template = Template('nitroxy.xml')
		template.rasterize(dst=dst, size=size, params=kwargs)
	
	def raster_is_valid(reference, resolution, **kwargs):
		return reference == resolution
