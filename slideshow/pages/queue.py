#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy
from slideshow.lib import queue, template

class Ajax(object):
    @cherrypy.expose
    def rename(self, id, value):
        id = int(id[6:]) # remove prefix
        c = cherrypy.thread_data.db
        q = queue.from_id(c, id)
        
        if q is None:
            return ''
        
        q.rename(c, value)
        cherrypy.thread_data.db.commit()
        return value
    
    @cherrypy.expose
    def remove(self, id):
        id = int(id)
        
        c = cherrypy.thread_data.db
        if not queue.delete(c, id):
            return 'false'
        
        cherrypy.thread_data.db.commit()
        return 'true'

class Handler(object):
    ajax = Ajax()
    
    @cherrypy.expose
    @template.output('queue/index.html', parent='queues')
    def index(self):
        queues = queue.all(cherrypy.thread_data.db)
        return template.render(queues=queues)
    
    @cherrypy.expose
    def add(self, name, submit):
        name = name.decode('utf-8')
        c = cherrypy.thread_data.db
        queue.add(c, name)
        cherrypy.thread_data.db.commit()
        raise cherrypy.HTTPRedirect('/queue')
    
    @cherrypy.expose
    @template.output('queue/rename.html', parent='queues')
    def rename(self, id, name=None, submit=None):
        c = cherrypy.thread_data.db
        q = queue.from_id(c, id)
        
        if name is not None:
            q.rename(c, name)
            cherrypy.thread_data.db.commit()
            raise cherrypy.HTTPRedirect('/queue')
        
        return template.render(queue=q)
    
    @cherrypy.expose
    def activate(self, id):
        queue.activate(id)
        raise cherrypy.HTTPRedirect('/')
    
    @cherrypy.expose
    def loop(self, id, state):
        queue.set_loop(id, state)
        raise cherrypy.HTTPRedirect('/')
