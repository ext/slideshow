#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os.path
import assembler as asm

image_path = '.'

class Slide:
	def __init__(self, id, queue, path, active, assembler, data):
		self.id = id
		self._queue = queue
		self._path = path
		self.active = active
		self.assembler = asm.get(assembler)
		self._data = data
		
		if not os.path.exists(path):
			raise ValueError, "could not locate '{path}' in '{root}'".format(path=path, root=image_path)
	
	def default_size(self, width=None):
		return self.assembler.default_size(slide=self, src=self._data, width=width)
	
	def raster_path(self, size):
		return os.path.join(image_path, self._path, 'raster', '%dx%d.png' % size)
	
	def src_path(self, item=None):
		return os.path.join(image_path, self._path, 'src', item)
	
	def _has_raster(self, size):
		return os.path.exists(self.raster_path(size))
	
	def rasterize(self, size):
		if not self._has_raster(size):
			self.assembler.rasterize(slide=self, src=self._data, size=size)

def from_id(c, id):
	row = c.execute("""
		SELECT
			id,
			path,
			active,
			assembler,
			data
		FROM
			slide
		WHERE
			id = :id
		LIMIT 1
	""", {'id': id}).fetchone()
	return Slide(queue=None, **row)
