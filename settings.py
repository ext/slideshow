#!/usr/bin/env python
# -*- coding: utf-8 -*-

from xml.dom import minidom
import os, os.path, stat, traceback
import json, xquery

try:
	# python 3.0
	import html.entities as entities
except ImportError:
	import htmlentitydefs  as entities

# Works like a dictionary but preserves order (well it's more like a list but
# with lookup.
class OrderDict:
	def __init__(self):
		self._list = []
		self._dict = {}
	
	def __setitem__(self, k, v):
		self._list.append((k,v))
		self._dict[k] = v
	
	def __getitem__(self, k):
		return self._dict[k]

	def values(self):
		return [v for k,v in self._list]
	
	def keys(self):
		return [k for k,v in self._list]
	
	def items(self):
		return self._list

class Group:
	def __init__(self, name, description, hidden):
		self.name = name
		self.description = description
		self.hidden = hidden
		self.items = OrderDict()
	
	def add(self, item):
		self.items[item.name] = item
	
	def __getitem__(self, k):
		return self.items[k]
	
	def __iter__(self):
		return self.items.values().__iter__()

class Item:
	typename = '' # automatically set
	
	def __init__(self, group, name, title, description, rel):
		self.group = group
		self.name = name
		self.title = title
		self.description = description
		self.rel = rel
		self._value = self.default
	
	def _values(self):
		return dict(group=self.group.name, name=self.name, type=self.typename, value=unicode(self._value))
	
	def __str__(self):
		return '<input type="text" name="{group}.{name}" type="{type}" value="{value}"/>'.format(**self._values())
	
	def set(self, value, rollback=False):
		self._value = value
	
	def get(self):
		return self._value

class ItemDirectory(Item):
	default = ''
	
	def set(self, value, rollback=False):
		tmp = self._value
		self._value = value
		path = self.fullpath()
		
		try:
			try:
				statinfo = os.stat(path)
			except OSError:
				raise ValueError, 'Directory not found ' + path
			
			if not stat.S_ISDIR(statinfo.st_mode):
				raise ValueError, 'Not a directory'
			if not os.access(path, os.W_OK):
				raise ValueError, 'Need write-permission'
		except:
			if rollback:
				self._value = tmp
			raise
	
	def fullpath(self):
		# quick sanity check
		if self._value == '':
			return self._value
		
		# is no relative path is specified it is always considered absolute
		if not self.rel:
			return self._value
		
		# absolute path is not relative
		if self._value[0] == '/':
			return self._value
		
		# join relative path with it's parent
		return os.path.join(self.rel.fullpath(), self._value)

class ItemFile(Item):
	default = ''
	
	def fullpath(self):
		# quick sanity check
		if self._value == '':
			return self._value
		
		# is no relative path is specified it is always considered absolute
		if not self.rel:
			return self._value
		
		# absolute path is not relative
		if self._value[0] == '/':
			return self._value
		
		# join relative path with it's parent
		return os.path.join(self.rel.fullpath(), self._value)

class ItemString(Item):
	default = ''

class ItemInteger(Item):
	default = 0
	
	def set(self, value, rollback=False):
		tmp = self._value
		self._value = value
		
		try:
			try:
				self._value = int(value)
			except ValueError:
				raise ValueError, 'Not a valid integer'
		except:
			if rollback:
				self._value = tmp
			raise

class ItemFloat(Item):
	default = 0.0
	
	def set(self, value, rollback=False):
		tmp = self._value
		self._value = value
		
		try:
			try:
				self._value = float(value)
			except ValueError:
				raise ValueError, 'Not a valid float'
		except:
			if rollback:
				self._value = tmp
			raise

class ItemPassword(Item):
	default = ''
	
	def __str__(self):
		return '<input type="password" name="{group}.{name}" type="{type}" value=""/>'.format(**self._values())

_aspects = [
	(4, 3),   # Regular 4:3 (VGA, PAL, SVGA, etc)
	(3, 2),   # NTSC
	(5, 3),
	(5, 4),   # SXGA, QSXGA
	(16, 10), # 16:10 "Widescreen"
	(16, 9),  # 16:9 Widescreen
	(17, 9)
]
def _calc_aspect(w,h):
	for (x,y) in _aspects:
		if (w/x) == (h/y):
			return '%d:%d' % (x,y)
	return ''

_resolutions = [(w,h,r,_calc_aspect(w,h)) for (w,h,r) in xquery.resolutions(':0.1')]
_resolution_keys = ['{width}x{height}'.format(width=w, height=h, refresh=r) for (w,h,r,a) in _resolutions]
_resolution_names = ['{width}x{height} ({aspect})'.format(width=w, height=h, aspect=a) for (w,h,r,a) in _resolutions]

class ItemResolution(Item):
	default = None
	values = zip(_resolution_keys, _resolution_names)
	
	def __str__(self):
		head = '<select name="{group}.{name}">'.format(**self._values())
		content = '\n'.join(['<option value="{key}">{value}</option>'.format(key=k, value=v) for k,v in self.values])
		tail = '</select>'
		
		return head + content + tail

class ItemDisplay(Item):
	default = ':0.0'
	values = xquery.screens()
	
	def __str__(self):
		head = '<select name="{group}.{name}">'.format(**self._values())
		content = '\n'.join(['<option value="{v}">{v}</option>'.format(v=v) for v in self.values])
		tail = '</select>'
		
		return head + content + tail

itemfactory = {
	'directory': ItemDirectory,
	'file':      ItemFile,
	'string':    ItemString,
	'integer':   ItemInteger,
	'float':     ItemFloat,
	'password':  ItemPassword,
	'resolution':ItemResolution,
	'display':   ItemDisplay
}

for k,v in itemfactory.items():
	v.typename = k

class Settings:
	def __init__(self, base, config_file=None):
		doc = minidom.parse(base)
		self.groups = OrderDict()
		self.enviroment = []
		self.config_file = config_file
		
		for group in doc.getElementsByTagName('group'):
			grpname = group.getAttribute('name')
			hidden  = group.hasAttribute('hidden') and group.getAttribute('hidden') == '1'
			grpdesc = group.getElementsByTagName('description')[0].childNodes[0].data
			
			g = Group(name=grpname, description=grpdesc, hidden=hidden)
			
			for item in group.getElementsByTagName('item'):
				name  = item.getAttribute('name')
				title = item.getAttribute('title')
				desc  = item.getAttribute('description')
				type  = item.hasAttribute('type') and item.getAttribute('type') or None
				rel   = item.hasAttribute('rel') and item.getAttribute('rel') or None # deferred resolving
				
				item = itemfactory[type](g, name, title, desc, rel)
				g.add(item)
			
			self.groups[grpname] = g
		
		# resolve relative paths
		for group in self:
			for item in group:
				if not item.rel:
					continue
				
				item.rel = self[item.rel]
		
		# load stored settings
		with open(config_file, 'r') as f:
			c = json.load(f)
			for k,v in c.items():
				if k == 'Env':
					self.enviroment = v
					continue
				try:
					group = self.groups[k]
				except KeyError:
					print 'group', k, 'found in config but is not defined xml'
					continue
				
				for name, value in v.items():
					try:
						item = group[name]
						item.set(value, rollback=True)
					except ValueError as e:
						# @todo sometimes they depend on settings not yet found and fail because of that.
						# e.g. path1 depends on path2, but path1 is set before path2 is set, so it fails.
						print name, k, 'contains illegal data ("%s": %s), resetting to default' % (value, str(e))
					except KeyError:
						print name, k, 'found in config but is not defined in xml'
			
	def __iter__(self):
		return self.groups.values().__iter__()
	
	def item(self, key):
		[groupname, itemname] = key.split('.')
		return self.groups[groupname][itemname]
	
	def __getitem__(self, key):
		if key == 'Env':
			return self.enviroment
		
		item = self.item(key)
		
		return item._value
	
	def __setitem__(self, key, value):
		if key == 'Env':
			self.enviroment = value
			return
		
		item = self.item(key)
		
		item.set(value)
	
	def persist(self, dst=None):
		c = {}
		for group in self:
			d = {}
			for item in group:
				d[item.name] = item.get()
			c[group.name] = d
		c['Env'] = self.enviroment
		
		with open(self.config_file, 'w') as fp:
			json.dump(c, fp, indent=4)
