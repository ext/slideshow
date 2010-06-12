#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import queue, slide, template

class Handler(object):
	@cherrypy.expose
	@template.output('maintenance/index.html')
	def index(self):
		return template.render()
