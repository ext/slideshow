#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import cherrypy
import sqlite3
import re
import slideshow
from slideshow.lib import template
from slideshow.pages import slides
from slideshow.pages import maintenance
from slideshow.pages import queue
from slideshow.settings import Settings
import slideshow.daemon as daemon

class Root(object):
	slides = slides.Handler()
	maintenance = maintenance.Handler()
	queue = queue.Handler()
	
	@cherrypy.expose
	def index(self):
		raise cherrypy.InternalRedirect('/slides/list')

class Browser:
	def __init__(self, host, username, password, name):
		self._host = host
		self._username = username
		self._password = password
		self._name = name

	@staticmethod
	def from_string(string):
		m = re.match('([a-z]+)://(.+)', string)
		if m is None:
			raise RuntimeError, 'string %s is not a valid browser string!' % string

		# @todo factory
		vendor = m.group(1)
		host = m.group(2)
		if vendor == 'sqlite':
			return SQLite3(host, None, None, None)
		else:
			raise RuntimeError, 'Unsupported browser %s' % str(vendor)

class SQLite3(Browser):
	def __init__(self, *args, **kwargs):
		Browser.__init__(self, *args, **kwargs)

		conn = self._connect()

		# check if not previous database is created
		row = conn.execute('SELECT COUNT(*) FROM sqlite_master WHERE name = \'slide\'').fetchone()[0]
		if not row or row == 0:
			# install database schema
			filename = os.path.join(os.path.dirname(slideshow.__file__), 'install', 'sqlite.sql')
			lines = "\n".join(open(filename, 'r').readlines())
			conn.executescript(lines)

	def connect(self, *args):
		cherrypy.thread_data.db = self._connect()

	def _connect(self):
		conn = sqlite3.connect(self._host)
		conn.row_factory = sqlite3.Row
		conn.cursor().execute('PRAGMA foreign_keys = ON')
		return conn
	
def run(config_file=None, browser_string='sqlite://site.db'):
	browser = Browser.from_string(browser_string)

	# make all worker threads connect to the database
	cherrypy.engine.subscribe('start_thread', browser.connect)

	if config_file and not os.path.exists(config_file):
		raise OSError, 'could not open config_file: %s', config_file

	if not config_file:
		print 'Warning! No config file specified, will use defaults'
		config_file = {}

	# load application config
	application = cherrypy.tree.mount(Root(), '/', config=config_file)
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
			})
	
	# cherrypy site config
	cherrypy.config.update({'sessionFilter.on': True})
	cherrypy.config.update(config_file) 

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
	run(*sys.argv[1:])
