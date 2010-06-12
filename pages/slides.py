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
	#@cherrypy.tools.response_headers(headers=[('Content-Type', 'image/png')])
	def show(self, id, width=200, height=200):
		s = slide.from_id(cherrypy.thread_data.db.cursor(), id)
		return str(s._has_raster((width,height)))
	
	@cherrypy.expose
	@template.output('slides/upload.html', parent='slides')
	def upload(self):
		return template.render()
