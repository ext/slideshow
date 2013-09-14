import os
from os.path import join, dirname

class Browser:
    # registered factories
    factory = {}

    def __init__(self, hostname, username, password, database):
        self.provider = self.__class__.provider
        self.hostname = hostname
        self.username = username
        self.password = password
        self.database = database

    def install(self):
        import slideshow
        directory = join(dirname(slideshow.__file__), 'install')
        for filename in [self.__class__._install, 'common.sql']:
            lines = open(join(directory, filename), 'r').read()
            self.executescript(lines)

    def __str__(self):
        return self.string(password=False)

    def have_password(self):
        return len(self.password) > 0

    def string(self, password=False):
        credentials = ''
        if self.username != '':
            credentials = self.username

            # if plain-text password is enable
            if password and self.password != '':
                credentials += ':' + self.password

            credentials += '@'

        hostname = self.hostname
        if hostname != '':
            hostname += '/'

        return '{provider}://{credentials}{hostname}{name}'.format(provider=self.provider, credentials=credentials, hostname=hostname, name=self.database)

    def __enter__(self):
        self.transaction()

    def __exit__(self, exc_type, exc_value, traceback):
        if exc_type is None:
            self.commit()
        else:
            self.rollback()

def register(name):
    def inner(cls):
        Browser.factory[name] = cls
        cls.provider = name
        return cls
    return inner

def on_install(name):
    def inner(cls):
        cls._install = name
        return cls
    return inner

def from_string(string):
    m = re.match('([a-z]+)://(.+)', string)
    if m is None:
        raise RuntimeError, 'string %s is not a valid browser string!' % string

    provider = m.group(1)
    name = m.group(2)

    Browser.factory[provider](None, None, None, name)

def from_settings(settings):
    provider = settings['Database.Provider']
    username = settings['Database.Username']
    password = settings['Database.Password']
    hostname = settings['Database.Hostname']
    database = settings['Database.Name']

    return Browser.factory[provider](hostname, username, password, database)

have_sqlite = True # in stdlib
have_mysql = False

# test if mysql is available
try:
    import MySQLdb
    have_mysql = True
except ImportError:
    pass

if have_mysql:  import slideshow.lib.browser._mysql
if have_sqlite: import slideshow.lib.browser._sqlite3
