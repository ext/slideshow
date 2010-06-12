#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, os.path
import assembler as asm
import shutil, uuid

image_path = os.path.expanduser('~/slideshow/image')

class InvalidSlide(Exception):
	pass

class Slide:
	def __init__(self, id, queue, path, active, assembler, data, stub=False):
		self.id = id
		self._queue = queue
		self._path = path
		self.active = active
		self.assembler = asm.get(assembler)
		self._data = data
		
		# Tells if this is a full slide (which database entry) or if it is being
		# constructed or is otherwise not complete.
		self._stub = stub
		
		if not os.path.exists(path):
			raise ValueError, "could not locate '{path}' in '{root}'".format(path=path, root=image_path)
	
	def assemble(self, params):
		return self.assembler.assemble(self, **params)
	
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
	
	if not row:
		raise InvalidSlide, "No slide with id ':id'".format(id=id)
	
	return Slide(queue=None, stub=False, **row)

def create(c, assembler, params):
	name = '{uuid}.slide'.format(uuid=uuid.uuid1().hex)
	dst = os.path.join(image_path, name)
	
	os.mkdir(dst)
	os.mkdir(os.path.join(dst, 'raster'))
	os.mkdir(os.path.join(dst, 'src'))
	
	slide = Slide(id=None, queue=None, path=dst, active=False, assembler=assembler, data=None, stub=True)
	data = slide.assemble(params)
	print 'data:', data
	
	c.execute("""
		INSERT INTO slide (
			queue_id,
			path,
			assembler,
			data
		) VALUES (
			1,
			:path,
			:assembler,
			:data
		)
	""", dict(path=slide._path, assembler=slide.assembler.name, data=data))
	
	return slide

def delete(c, id):
	s = from_id(c, id)
	
	c.execute("""
		DELETE FROM
			slide
		WHERE
			id = :id
	""", dict(id=s.id))
	
	shutil.rmtree(s._path)
