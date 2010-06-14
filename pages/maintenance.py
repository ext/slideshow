#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import queue, slide, template
from settings import Settings

class Handler(object):
	@cherrypy.expose
	@template.output('maintenance/index.html', parent='maintenance')
	def index(self):
		return template.render()

	@cherrypy.expose
	@template.output('maintenance/config.html', parent='maintenance')
	def config(self, action=None, **kwargs):
		settings = Settings('settings.xml', 'settings.json')
		
		if action == 'save':
			for k,v in kwargs.items():
				settings[k] = v
			
			settings.persist()
			raise cherrypy.HTTPRedirect('/maintenance/config')
		
		return template.render(settings=settings)
