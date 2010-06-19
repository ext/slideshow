#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import queue, slide, template
import daemon

class Handler(object):
	@cherrypy.expose
	@template.output('slides/view.html')
	def list(self):
		queues = queue.all(cherrypy.thread_data.db.cursor())
		return template.render(queues=queues)
	
	@cherrypy.expose
	@cherrypy.tools.response_headers(headers=[('Content-Type', 'image/png')])
	def show(self, id, *args):
		s = slide.from_id(cherrypy.thread_data.db.cursor(), id)
		
		if len(args) == 0:
			size = s.default_size()
		elif len(args) == 1:
			size = s.default_size(args[0])
		else:
			size = (int(args[0]), int(args[1]))
		
		s.rasterize(size)
		
		# @todo static.serve_file
		f = open(s.raster_path(size), 'rb')
		bytes = f.read()
		f.close()
		
		return bytes
	
	@cherrypy.expose
	@template.output('slides/upload.html', parent='slides')
	def upload(self):
		return template.render()

	@cherrypy.expose
	def submit(self, assembler, **kwargs):
		try:
			s = slide.create(cherrypy.thread_data.db.cursor(), assembler, kwargs)
			daemon.ipc.Reload()
			cherrypy.thread_data.db.commit()
		except:
			cherrypy.thread_data.db.rollback()
			raise
		raise cherrypy.HTTPRedirect('/')
	
	@cherrypy.expose
	def delete(self, id):
		try:
			slide.delete(cherrypy.thread_data.db.cursor(), id)
			cherrypy.thread_data.db.commit()
		except:
			cherrypy.thread_data.db.rollback()
			raise
		raise cherrypy.HTTPRedirect('/')
