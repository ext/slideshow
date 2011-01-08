#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import cherrypy
import sqlite3
from slideshow.lib import template
from slideshow.pages import slides
from slideshow.pages import maintenance
from slideshow.pages import queue
from slideshow.settings import Settings
import slideshow.daemon as daemon

def connect(*args):
	cherrypy.thread_data.db = sqlite3.connect('site.db')
	cherrypy.thread_data.db.row_factory = sqlite3.Row
	cherrypy.thread_data.db.cursor().execute('PRAGMA foreign_keys = ON')

class Root(object):
	slides = slides.Handler()
	maintenance = maintenance.Handler()
	queue = queue.Handler()
	
	@cherrypy.expose
	def index(self):
		raise cherrypy.InternalRedirect('/slides/list')

def run():
	# make all worker threads connect to the database
	cherrypy.engine.subscribe('start_thread', connect)

	# load cherrypy config
	application = cherrypy.tree.mount(Root(), '/', config='test.conf')
	application.config.update({
			'/': {
				'tools.staticdir.root': os.path.dirname(os.path.abspath(__file__)),
				'tools.gzip.on': True,
				'tools.encode.on': True,
				'tools.encode.encoding': 'utf8',
				},
			'/static': {
				'tools.staticdir.on': True,
				'tools.staticdir.dir': '../static',
				},
			'sessionFilter.on': True,
			})
	

	# load slideshow settings
	settings = Settings()
	settings.load('settings.xml', 'settings.json')
	
	# let daemon subscribe to cherrypy events to help stopping daemon when cherrypy is terminating
	daemon.subscribe(cherrypy.engine)
	
	# start frontend
	if hasattr(cherrypy.engine, 'block'):
		# 3.1 syntax
		cherrypy.engine.start()
		cherrypy.engine.block()
	else:
		# 3.0 syntax
		cherrypy.server.quickstart()
		cherrypy.engine.start()
		
if __name__ == '__main__':
	run()
