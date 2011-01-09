import os
import sqlite3
import cherrypy
from slideshow.lib.browser import Browser, register
from slideshow.settings import Settings

@register('sqlite3')
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
		settings = Settings()
		filename = os.path.join(settings['Path.BasePath'], self._name)
		
		try:
			conn = sqlite3.connect(filename)
		except Exception, e:
			raise IOError, '%s: %s' % (e, filename)

		conn.row_factory = sqlite3.Row
		conn.cursor().execute('PRAGMA foreign_keys = ON')
		return conn
