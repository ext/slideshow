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
    @template.output('queue/rename.html', parent='queue')
    def rename(self, id, name=None, submit=None):
        c = cherrypy.thread_data.db.cursor()
        q = queue.from_id(c, id)
        
        if name is not None:
            q.rename(c, name)
            cherrypy.thread_data.db.commit()
            raise cherrypy.HTTPRedirect('/queue')
        
        return template.render(queue=q)
    
    @cherrypy.expose
    def activate(self, id):
        settings = Settings()
        
        with settings:
            settings['Runtime.queue'] = id
        
        settings.persist()
        
        raise cherrypy.HTTPRedirect('/')
