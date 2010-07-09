#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import template
from settings import Settings

class Handler(object):
    @cherrypy.expose
    def activate(self, id):
        settings = Settings()
        
        with settings:
            settings['Runtime.queue'] = id
        
        settings.persist()
        
        raise cherrypy.HTTPRedirect('/')
