#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import queue, slide, template

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
		
		f = open(s.raster_path(size), 'rb')
		bytes = f.read()
		f.close()
		
		return bytes
	
	@cherrypy.expose
	@template.output('slides/upload.html', parent='slides')
	def upload(self):
		return template.render()
