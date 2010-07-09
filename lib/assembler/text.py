#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
from lib.resolution import Resolution
from settings import Settings
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
		
		resolution = params['resolution']
		realsize = resolution.fit(size)
		
		# scale constant
		scale = float(realsize.w) / resolution.w
		
		data = array.array('c', chr(0) * int(size.w) * int(size.h) * 4)
		surface = cairo.ImageSurface.create_for_data(data, cairo.FORMAT_ARGB32, int(size.w), int(size.h), int(size.w)*4)
		cr = cairo.Context(surface)
		
		cr.translate((size.w-realsize.w)*0.5, (size.h-realsize.h)*0.5)
		
		cr.save()
		cr.set_source_rgba(0.5, 0.0 , 0.5, 0.3)
		cr.set_operator(cairo.OPERATOR_SOURCE)
		cr.paint()
		
		cr.set_source_rgba(0, 0, 0, 1)
		cr.rectangle(0, 0, realsize.w, realsize.h)
		cr.fill()
		cr.restore()
		
		background = template.getAttribute('background')
		if background:
			image = cairo.ImageSurface.create_from_png(background)
			w = image.get_width()
			h = image.get_height()
			
			cr.save()
			cr.scale(realsize.w/w, realsize.h/h)
			cr.set_source_surface(image, 0, 0)
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
		
		# write_to_png doesn't accept unicode string, yet
		if isinstance(dst, basestring):
			dst = str(dst)
		
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
	def default_size(self, slide, params, width=None):
		resolution = Settings().resolution()
		if width:
			print resolution.aspect()
			print resolution.scale(width=width).aspect()
			return resolution.scale(width=width)
		else:
			return resolution
	
	def is_editable(self):
		return True
	
	def assemble(self, slide, **kwargs):
		kwargs = kwargs.copy()
		kwargs['resolution'] = (kwargs['resolution'].w, kwargs['resolution'].h)
		return kwargs
	
	def rasterize(self, size, params, slide=None, file=None):
		params = params.copy()
		params['resolution'] = Resolution(params['resolution'][0], params['resolution'][1])
		
		dst = slide and slide.raster_path(size) or file
		template = Template('nitroxy.xml')
		template.rasterize(dst=dst, size=size, params=params)
	
	def raster_is_valid(reference, resolution, **kwargs):
		return reference == resolution
	
	def title(self):
		return 'Text'
