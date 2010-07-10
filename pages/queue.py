#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from lib import queue, template
from settings import Settings

class Handler(object):
    @cherrypy.expose
    @template.output('queue/index.html', parent='queue')
    def index(self):
        queues = queue.all(cherrypy.thread_data.db.cursor())
        return template.render(queues=queues)
    
    @cherrypy.expose
    def activate(self, id):
        settings = Settings()
        
        with settings:
            settings['Runtime.queue'] = id
        
        settings.persist()
        
        raise cherrypy.HTTPRedirect('/')
