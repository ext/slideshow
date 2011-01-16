#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, os.path, json
import slideshow.lib.assembler as asm
import shutil, uuid
from slideshow.settings import Settings
from slideshow.lib.resolution import Resolution
import slideshow.event as event
import cherrypy

class InvalidSlide(Exception):
	pass

class Slide:
	def __init__(self, id, queue, path, active, assembler, data, stub=False, validate_path=True):
		self.id = id
		self._queue = queue
		self._path = path
		self.active = active
		self.assembler = asm.get(assembler)
		self._data = data and json.loads(data) or None
		
		# Tells if this is a full slide (which database entry) or if it is being
		# constructed or is otherwise not complete.
		self._stub = stub

		if validate_path and not os.path.exists(path):
			settings = Settings()
			base_path = settings['Path.BasePath']
			image_path = settings['Path.Image']
			raise ValueError, "could not locate '{path}' in '{root}'".format(path=path, root=os.path.join(base_path, image_path))
	
	def assemble(self, params):
		return json.dumps(self.assembler.assemble(self, **params))
	
	def default_size(self, width=None):
		return self.assembler.default_size(slide=self, params=self._data, width=width)
	
	def raster_path(self, size=None):
		settings = Settings()
		base_path = settings['Path.BasePath']
		image_path = settings['Path.Image']
		
		args = [base_path, image_path, self._path, 'raster']
		if size != None:
			args.append(str(size) + self.assembler.raster_extension())
		return os.path.join(*args)
	
	def src_path(self, item):
		settings = Settings()
		base_path = settings['Path.BasePath']
		image_path = settings['Path.Image']
		
		return os.path.join(base_path, image_path, self._path, 'src', item)
	
	def _has_raster(self, size):
		return os.path.exists(self.raster_path(size))
	
	def rasterize(self, size):
		"""
		Rasterizes the slide for the given resolution, if needed.
		:param size: Size of the raster
		"""
		if not self._has_raster(size):
			self.assembler.rasterize(slide=self, size=size, params=self._data)
	
	def _invalidate(self):
		path = self.raster_path()
		for root, dirs, files in os.walk(path):
			[os.remove(os.path.join(root,x)) for x in files]
	
	def rebuild_cache(self, resolution):
		self._invalidate()
		self.rasterize(Resolution(200,200)) # thumbnail
		self.rasterize(Resolution(800,600)) # windowed mode (debug)
		self.rasterize(resolution)

	def switch(self, c, dst):
		head, tail = os.path.split(self._path)
		self._path = os.path.join(dst, 'image', tail)
		c.execute('UPDATE slide SET path = :path WHERE id = :id', dict(path=self._path, id=self.id))
		return self._path

def all(c, validate_path=True):
	return [Slide(queue=None, validate_path=validate_path, **x) for x in c.execute("""
		SELECT
			id,
			path,
			active,
			assembler,
			data
		FROM
			slide
		""").fetchall()]

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

	base_path = settings['Path.BasePath']
	image_path = settings['Path.Image']

	name = '{uuid}.slide'.format(uuid=uuid.uuid1().hex)
	dst = os.path.join(base_path, image_path, name)
	
	os.mkdir(dst)
	os.mkdir(os.path.join(dst, 'raster'))
	os.mkdir(os.path.join(dst, 'src'))
	
	# reference resolution
	params['resolution'] = settings.resolution()
	
	slide = Slide(id=None, queue=None, path=dst, active=False, assembler=assembler, data=None, stub=True)
	slide._data = json.loads(slide.assemble(params))
	slide.rebuild_cache(settings.resolution())
	
	c.execute("""
		INSERT INTO slide (
			queue_id,
			path,
			assembler,
			data
		) VALUES (
			0,
			:path,
			:assembler,
			:data
		)
	""", dict(path=slide._path, assembler=slide.assembler.name, data=json.dumps(slide._data)))
	
	return slide

def edit(c, id, assembler, params):
	settings = Settings()
	
	# reference resolution
	params['resolution'] = settings.resolution()
	
	slide = from_id(c, id)
	slide._data = json.loads(slide.assemble(params))
	slide.rebuild_cache(settings.resolution())
	
	c.execute("""
		UPDATE
			slide
		SET
			data = :data
		WHERE
			id = :id
	""", dict(id=id, data=json.dumps(slide._data)))

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
	@event.callback('maintenance.rebuild')
	def flush(self, progresss):
		c = cherrypy.thread_data.db.cursor()
		settings = Settings()
		
		slides = [Slide(queue=None, **x) for x in c.execute("""
			SELECT
				id,
				path,
				active,
				assembler,
				data
			FROM
				slide
		""").fetchall()]
		
		for n, slide in enumerate(slides):
			progresss(str(float(n+1) / len(slides) * 100) + '%<br/>\n')
			slide.rebuild_cache(settings.resolution())
	
	@event.callback('config.resolution_changed')
	def resolution_changed(self, resolution):
		c = cherrypy.thread_data.db.cursor()
		
		slides = [Slide(queue=None, **x) for x in c.execute("""
			SELECT
				id,
				path,
				active,
				assembler,
				data
			FROM
				slide
		""").fetchall()]
		
		for slide in slides:
			params = slide._data
			params['resolution'] = resolution
			
			slide._data = json.loads(slide.assemble(params))
			slide.rebuild_cache(resolution)
			
			c.execute("""
				UPDATE
					slide
				SET
					data = :data
				WHERE
					id = :id
			""", dict(id=slide.id, data=json.dumps(slide._data)))
		
		cherrypy.thread_data.db.commit()

_listener = EventListener()
