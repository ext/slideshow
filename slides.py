#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import template

class Handler(object):
	@cherrypy.expose
	@template.output('slides/view.html')
	def view(self):
		return template.render()
	
	@cherrypy.expose
	@template.output('slides/upload.html')
	def upload(self):
		return template.render()
