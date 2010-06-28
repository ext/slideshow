#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, os.path, json
import assembler as asm
import shutil, uuid
from settings import Settings
import event

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
		self._data = data and json.loads(data) or None
		
		# Tells if this is a full slide (which database entry) or if it is being
		# constructed or is otherwise not complete.
		self._stub = stub
		
		if not os.path.exists(path):
			raise ValueError, "could not locate '{path}' in '{root}'".format(path=path, root=image_path)
	
	def assemble(self, params):
		return json.dumps(self.assembler.assemble(self, **params))
	
	def default_size(self, width=None):
		return self.assembler.default_size(slide=self, src=self._data, width=width)
	
	def raster_path(self, size):
		return os.path.join(image_path, self._path, 'raster', '%dx%d.png' % size)
	
	def src_path(self, item):
		return os.path.join(image_path, self._path, 'src', item)
	
	def _has_raster(self, size):
		return os.path.exists(self.raster_path(size))
	
	def rasterize(self, size):
		"""
		Rasterizes the slide for the given resolution, if needed.
		:param size: Size of the raster
		"""
		if not self._has_raster(size):
			self.assembler.rasterize(slide=self, size=size, **self._data)

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
	settings = Settings()
	name = '{uuid}.slide'.format(uuid=uuid.uuid1().hex)
	dst = os.path.join(image_path, name)
	
	os.mkdir(dst)
	os.mkdir(os.path.join(dst, 'raster'))
	os.mkdir(os.path.join(dst, 'src'))
	
	# reference resolution
	params['resolution'] = settings.resolution()
	
	slide = Slide(id=None, queue=None, path=dst, active=False, assembler=assembler, data=None, stub=True)
	slide._data = json.loads(slide.assemble(params))
	
	slide.rasterize((200,200)) # thumbnail
	slide.rasterize((800,600)) # windowed mode (debug)
	slide.rasterize(settings.resolution())
	
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
	""", dict(path=slide._path, assembler=slide.assembler.name, data=json.dumps(slide._data)))
	
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

@event.listener
class EventListener:
	@event.callback('config.resolution_changed')
	def resolution_changed(*args, **kwargs):
		print 'resolution is changed', args, kwargs

_listener = EventListener()
