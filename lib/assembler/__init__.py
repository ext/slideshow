#!/usr/bin/env python
# -*- coding: utf-8 -*-

from genshi.template import TemplateLoader
from lib import template
import os.path

loader = TemplateLoader(
	os.path.join(os.path.dirname(__file__)),
	auto_reload=True
)

class Assembler:
	name = '' # automatically set in factory initialization
	
	def is_editable(self):
		"""
		Tells whenever an assembler is editable or not.
		"""
		return False
	
	def assemble(self, slide, **kwargs):
		"""
		Prepares an assembler. It does not rasterize anything.
		"""
		raise NotImplementedError
	
	def rasterize(self, slide, src, size, params):
		"""
		Rasterize a slide to the given resolution. It takes the source data
		earlier prepared (using assemble) and creates an image of the given
		resolution.
		"""
		raise NotImplementedError
	
	def default_size(self, slide, params, width=None):
		"""
		It returns the default size for this slide. Eg for an image slide it
		returns the resolution of the source image, and for a text slide it
		gives the configured resolution.
		"""
		raise NotImplementedError
	
	def raster_is_valid(**kwargs):
		"""
		Determines whenever the cached raster is valid
		"""
		return True
	
	def title(self):
		"""
		Get a pretty name of this type of assembler
		"""
		raise NotImplementedError
	
	def render(self, content):
		"""
		Get html representation of the upload/edit form (should include the
		fieldset wrapping the fields)
		"""
		
		func = lambda: template.render(**content)
		file = self.name + '.html'
		return template.output(file, doctype=False, loader=loader)(func)()

import image
import text

_assemblers = {
	'text': text.TextAssembler,
	'image': image.ImageAssembler
}

# setup reverse names
for k,v in _assemblers.items():
	v.name = k

def get(name):
	return _assemblers[name]()

def all():
	return dict([(k,v()) for k,v in _assemblers.items()])
