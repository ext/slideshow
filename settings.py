#!/usr/bin/env python
# -*- coding: utf-8 -*-

from xml.dom import minidom
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
	
	def __init__(self, group, name, title, description):
		self.group = group
		self.name = name
		self.title = title
		self.description = description
		self.value = self.default
	
	def _values(self):
		return dict(group=self.group.name, name=self.name, type=self.typename, value=unicode(self.value))
	
	def __str__(self):
		return '<input type="text" name="{group}.{name}" type="{type}" value="{value}"/>'.format(**self._values())

class ItemDirectory(Item):
	default = ''

class ItemFile(Item):
	default = ''

class ItemString(Item):
	default = ''

class ItemInteger(Item):
	default = 0

class ItemFloat(Item):
	default = 0.0

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
				
				item = itemfactory[type](g, name, title, desc)
				g.add(item)
			
			self.groups[grpname] = g
		
		# load stored settings
		with open(config_file, 'r') as f:
			c = json.load(f)
			for k,v in c.items():
				if k == 'Env':
					self.enviroment = v
				try:
					group = self.groups[k]
				except KeyError:
					print 'group', k, 'found in config but is not defined xml'
					continue
				
				for name, value in v.items():
					try:
						item = group[name]
						item.value = value
					except KeyError:
						print name, k, 'found in config but is not defined in xml'
			
	def __iter__(self):
		return self.groups.values().__iter__()
	
	def __getitem__(self, key):
		if key == 'Env':
			return self.enviroment
		
		print key
		[groupname, itemname] = key.split('.')
		item = self.groups[groupname][itemname]
		
		return item
	
	def __setitem__(self, key, value):
		if key == 'Env':
			self.enviroment = value
			return
		
		item = self[key]
		item.value = value
	
	def persist(self, dst=None):
		c = {}
		for group in self:
			d = {}
			for item in group:
				d[item.name] = item.value
			c[group.name] = d
		c['Env'] = self.enviroment
		
		with open(self.config_file, 'w') as fp:
			json.dump(c, fp, indent=4)
