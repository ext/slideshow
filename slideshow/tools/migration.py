#!/usr/bin/env python
# -*- coding: utf-8 -*-

from slideshow.settings import Settings
from cherrypy.process.plugins import SimplePlugin
from glob import glob
from os.path import basename, dirname, join, realpath
import sys
import traceback

class Migration(SimplePlugin):
    def __init__(self, bus, browser):
        SimplePlugin.__init__(self, bus)
        self.browser = browser

    def start(self):
        db = self.browser

        # ensure migration table exists
        try:
            db.execute('SELECT 1 FROM `migration`').fetchone()
        except:
            self.bus.log("Migration table doesn't exist, creating it.")
            db.execute("""
                CREATE TABLE `migration` (
                    `filename` VARCHAR(256) PRIMARY KEY,
                    `timestamp` TIMESTAMP NOT NULL
                );""")

        # generate a list of all migrations that hasn't been executed
        path = realpath(join(dirname(__file__), '..', 'migrations', '*.sql'))
        migrations = filter(lambda x: not db.execute('SELECT 1 FROM `migration` WHERE `filename` = :filename', dict(filename=basename(x))).fetchone(), glob(path))
        migrations.sort()

        # test if there is any migrations to execute
        if len(migrations) == 0: return

        self.bus.log("Starting database migration.")
        for migration in migrations:
            self.bus.log('  Executing `%s\'' % migration)
            with open(migration) as fp:
                lines = fp.read()
            try:
                with db:
                    db.executescript(lines)
                    db.execute('INSERT INTO `migration` (`filename`, `timestamp`) VALUES (:filename, CURRENT_TIMESTAMP)', dict(filename=basename(migration)))
            except:
                traceback.print_exc()
                print >> sys.stderr, 'When executing migration `%s\'' % migration

    def stop(self):
        pass
