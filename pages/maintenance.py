#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import queue, slide, template
from settings import Settings

class Handler(object):
	@cherrypy.expose
	@template.output('maintenance/index.html')
	def index(self):
		return template.render()

	@cherrypy.expose
	@template.output('maintenance/config.html')
	def config(self, action=None):
		settings = Settings('settings.xml', 'settings.json')
		
		return template.render(settings=settings)
