class Browser:
	# registered factories
    	factory = {}

	def __init__(self, host, username, password, name):
		self._provider = self.__class__.provider
		self._host = host
		self._username = username
		self._password = password
		self._name = name

	def __str__(self):
		return self.string(password=False)

	def string(self, password=False):
		credentials = ''
		if self._username != '':
			credentials = self._username

			# if plain-text password is enable
			if password and self._password != '':
				credentials += ':' + self._password

			credentials += '@'

		hostname = self._host
		if hostname != '':
			hostname += '/'

		return '{provider}://{credentials}{hostname}{name}'.format(provider=self._provider, credentials=credentials, hostname=hostname, name=self._name)

def register(name):
    def inner(cls):
        Browser.factory[name] = cls
        cls.provider = name
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
    name = settings['Database.Name']
    
    return Browser.factory[provider](hostname, username, password, name)

import slideshow.lib.browser._sqlite3
