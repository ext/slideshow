import os
import sqlite3
import cherrypy
from slideshow.lib.browser import Browser, register, on_install
from slideshow.settings import Settings

@register('sqlite3')
@on_install('sqlite.sql')
class SQLite3(Browser):
    def __init__(self, *args, **kwargs):
        Browser.__init__(self, *args, **kwargs)

        self.real_connect()

        # check if not previous database is created
        row = self.execute('SELECT COUNT(*) FROM sqlite_master WHERE name = \'slide\'').fetchone()[0]
        if not row or row == 0:
            self.install()

    def real_connect(self):
        settings = Settings()
        filename = os.path.join(settings['Path.BasePath'], self.database)

        try:
            conn = sqlite3.connect(filename)
        except Exception, e:
            raise IOError, '%s: %s' % (e, filename)

        conn.row_factory = sqlite3.Row
        conn.cursor().execute('PRAGMA foreign_keys = ON')
        self.conn = conn
        self.cursor = conn.cursor()

    def fetchone(self):
        return self.cursor.fetchone()

    def fetchall(self):
        return self.cursor.fetchall()

    def execute(self, *args, **kwargs):
        return self.cursor.execute(*args, **kwargs)

    def executemany(self, *args, **kwargs):
        return self.cursor.executemany(*args, **kwargs)

    def executescript(self, lines):
        return self.conn.executescript(lines)

    def transaction(self):
        pass # autocommit is disabled

    def commit(self):
        self.conn.commit()

    def rollback(self):
        self.conn.rollback()

    def connect(self, *args):
        cherrypy.thread_data.db = SQLite3(self.hostname, self.username, self.password, self.database)
