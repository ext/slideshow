#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy, json
from slideshow.settings import Settings
import slideshow.daemon as daemon
import slideshow.event as event

class Handler(object):
	@cherrypy.expose
	def settings(self, name, format='raw'):
		print 'name:', name
		print 'format', format
		
		if format == 'raw':
			cherrypy.response.headers['Content-Type'] = 'application/json'
		elif format == 'text':
			cherrypy.response.headers['Content-Type'] = 'text/plain'
		
		settings = Settings()
		
		return json.dumps({
			'Queue': settings['Runtime.queue'],
			'TransitionTime': settings['Appearance.TransitionTime'],
			'SwitchTime': settings['Appearance.SwitchTime'],
		}, indent=4)
