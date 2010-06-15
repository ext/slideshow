#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import queue, slide, template
from settings import Settings
import daemon

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
			error = False
			
			for k,v in kwargs.items():
				try:
					settings[k] = v
				except ValueError as e:
					settings[k].message = e.message
					print settings[k].message, hasattr(settings[k], 'message')
					error = True
			
			if not error:
				settings.persist()
				raise cherrypy.HTTPRedirect('/maintenance/config')
		
		return template.render(settings=settings)
	
	@cherrypy.expose
	def start(self):
		daemon.start('', '')
		raise cherrypy.HTTPRedirect('/maintenance')
