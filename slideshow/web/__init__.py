#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys
import cherrypy
import sqlite3
import re
import argparse
import traceback

import slideshow
from slideshow.lib import template
from slideshow.pages import slides
from slideshow.pages import maintenance
from slideshow.pages import queue
from slideshow.settings import Settings
import slideshow.daemon as daemon

def get_resource_path(*path):
	return os.path.join(os.path.dirname(slideshow.__file__), *path)

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
			filename = get_resource_path('install', 'sqlite.sql')
			lines = "\n".join(open(filename, 'r').readlines())
			conn.executescript(lines)

	def connect(self, *args):
		cherrypy.thread_data.db = self._connect()

	def _connect(self):
		conn = sqlite3.connect(self._host)
		conn.row_factory = sqlite3.Row
		conn.cursor().execute('PRAGMA foreign_keys = ON')
		return conn

def run():
	# setup argument parser
	parser = argparse.ArgumentParser(description='Slideshow frontend')
	parser.add_argument('-f', '--config-file')
	parser.add_argument('-v', '--verbose', dest='verbose', action='store_true')
	parser.add_argument('-q', '--quiet', dest='verbose', action='store_false')
	parser.add_argument('-p', '--port', type=int, default=8000)
	parser.add_argument('--install', default=None)
	parser.add_argument('--browser', default='sqlite://site.db')

	# parse args
	args = parser.parse_args()

	try:
		# create new config if we are bootstraping
		if args.install:
			from os.path import isdir, join, exists

			# ensure path is a directory
			if not isdir(args.install):
				print >> sys.stderr, 'Must point to a directory, not', args.install
				sys.exit(1)

			for x in ['image', 'video', 'tmp']:
				os.mkdir(join(args.install, x))

			if not args.config_file:
				args.config_file = join(args.install, 'settings.conf')

			# check if config already exists
			if exists(args.config_file):
				print >> sys.stderr, 'Configuration already exists, cannot install a new one.'
				print >> sys.stderr, 'If you intend to replace the configuration, please remove the old one first, e.g. `rm %s\'' % args.config_file
				sys.exit(1)

			# put a dummy config, will be overwritten later
			try:
				with open(args.config_file, 'w') as f:
					f.write("{}\n")
			except IOError:
				print >> sys.stderr, 'Failed to install configuration at', args.config_file
				print >> sys.stderr, 'Ensure that you have write-permission to the specified location.'
				sys.exit(1)

			# load default settings
			settings = Settings()
			settings.load(get_resource_path('settings.xml'), None)
			
			# if creating a new config, the default config is persisted.
			if args.install:
				# store the directory we installed to
				with settings:
					settings['Path.BasePath'] = os.path.abspath(args.install)

				settings.persist(dst=args.config_file)
				print >> sys.stderr, 'Configuration installed, restart without --install flag to continue.'
				sys.exit(0)

		# verify that config_file exists
		if not os.path.exists(args.config_file):
			print >> sys.stderr, 'Could not open configuration file:', args.config_file
			print >> sys.stderr, 'You can create a new configuration by adding the --install flag or creating the file yourself.'
			print >> sys.stderr, 'Note that the file must be writable by the user running slideshow.'
			sys.exit(1)

		# verify that config_file is writable
		if not os.access(args.config_file, os.W_OK):
			print >> sys.stderr, 'Could not write to configuration file:', args.config_file
			print >> sys.stderr, 'Make sure the user has proper permissions.'
			sys.exit(1)

		# load slideshow settings
		settings = Settings()
		settings.load(get_resource_path('settings.xml'), args.config_file)

		# read cherrypy config
		config = settings['cherrypy']

		# cherrypy only accepts str, not unicode
		for k,v in config.items():
			if isinstance(v, basestring):
				config[k] = str(v)

		# load browser
		browser = Browser.from_string(args.browser)

		# make all worker threads connect to the database
		cherrypy.engine.subscribe('start_thread', browser.connect)

		# load application config
		application = cherrypy.tree.mount(Root(), '/')
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
		cherrypy.config.update({
				'sessionFilter.on': True,
				'server.socket_host': "0.0.0.0", 
				'server.socket_port': args.port
			})
		cherrypy.config.update(config)
	
		# let daemon subscribe to cherrypy events to help stopping daemon when cherrypy is terminating
		daemon.subscribe(cherrypy.engine)
	
		# start cherrypy
		if hasattr(cherrypy.engine, 'block'):
			# 3.1 syntax
			cherrypy.engine.start()
			cherrypy.engine.block()
		else:
			# 3.0 syntax
			cherrypy.server.quickstart()
			cherrypy.engine.start()

	except Exception, e: # don't leak any exceptions
		if args.verbose:
			traceback.print_exc()
		else:
			print >> sys.stderr, e
		
		# if this is reached the application crashed
		sys.exit(1)

	#
	#if not config_file:
	#	print 'Warning! No config file specified, will use defaults'
	#	config_file = {}
		
if __name__ == '__main__':
	run(*sys.argv[1:])
