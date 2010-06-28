#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import cherrypy
import sqlite3
from lib import template
from pages import slides
from pages import maintenance
from settings import Settings
import daemon

def connect(*args):
	cherrypy.thread_data.db = sqlite3.connect('site.db')
	cherrypy.thread_data.db.row_factory = sqlite3.Row
	cherrypy.thread_data.db.cursor().execute('PRAGMA foreign_keys = ON')

cherrypy.engine.subscribe('start_thread', connect)

class Root(object):
	slides = slides.Handler()
	maintenance = maintenance.Handler()
	
	@cherrypy.expose
	def index(self):
		raise cherrypy.InternalRedirect('/slides/list')

application = cherrypy.tree.mount(Root(), '/', config='test.conf')
application.config.update({
	'/': {
		'tools.staticdir.root': os.path.dirname(os.path.abspath(__file__)),
		'tools.encode.on':True,
		'tools.encode.encoding':'utf8',
	}
})

cherrypy.config.update({'sessionFilter.on': True}) 
settings = Settings()
settings.load('settings.xml', 'settings.json')

if __name__ == '__main__':
	#print daemon.instance
	cherrypy.config.update('test.conf')
	daemon.subscribe(cherrypy.engine)

	if hasattr(cherrypy.engine, 'block'):
	    # 3.1 syntax
	    cherrypy.engine.start()
	    cherrypy.engine.block()
	else:
	    # 3.0 syntax
	    cherrypy.server.quickstart()
	    cherrypy.engine.start()
