#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import cherrypy
import sqlite3
from lib import template
from pages import slides

def connect(*args):
	cherrypy.thread_data.db = sqlite3.connect('site.db')

cherrypy.engine.subscribe('start_thread', connect)

class Admin(object):
	pass

class Root(object):
	slides = slides.Handler()
	admin = Admin()
	
	@cherrypy.expose
	def index(self):
		raise cherrypy.InternalRedirect('/slides/view')

application = cherrypy.tree.mount(Root(), '/', config='test.conf')
application.config.update({
	'/': {
		'tools.staticdir.root': os.path.dirname(os.path.abspath(__file__)),
		'tools.encode.on':True,
		'tools.encode.encoding':'utf8',
	}
})

cherrypy.config.update({'sessionFilter.on': True}) 

if __name__ == '__main__':
	cherrypy.config.update('test.conf')

	if hasattr(cherrypy.engine, 'block'):
	    # 3.1 syntax
	    cherrypy.engine.start()
	    cherrypy.engine.block()
	else:
	    # 3.0 syntax
	    cherrypy.server.quickstart()
	    cherrypy.engine.start()
