#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy, os.path
from slideshow.lib import queue, slide, template, browser as browser_factory
from slideshow.settings import Settings, ItemCheckbox
import slideshow.daemon as daemon
import slideshow.event as event
import traceback

logcls = ['fatal', 'warning', 'info', 'verbose', 'debug']

class Ajax(object):
    @cherrypy.expose
    def log(self):
        return '<pre>' + '\n'.join(['<span class="%s">%s</span>' % (logcls[severity], line) for severity, line in daemon.log()]) + '</pre>'

class Handler(object):
    ajax = Ajax()

    @cherrypy.expose
    @template.output('maintenance/index.html', parent='maintenance')
    def index(self):
        settings = Settings()
        cmd, args, env, cwd = daemon.settings(browser_factory.from_settings(settings))
        return template.render(log=daemon.log(), logcls=logcls, state=daemon.state(), cmd=cmd, args=args, env=env, cwd=cwd)

    @cherrypy.expose
    @template.output('maintenance/config.html', parent='maintenance')
    def config(self, action=None, **kwargs):
        settings = Settings()

        if action == 'save':
            error = False
            with settings:
                # hack for environmental variables
                env = {}
                if 'Env' in kwargs:
                    try:
                        env = kwargs['Env'].split("\r\n")
                        env = filter(lambda x: len(x) > 0, env)
                        env = map(lambda x: tuple(x.split('=')), env)
                        env = dict(env)
                    except:
                        raise ValueError, 'Malformed environment variables'
                    finally:
                        kwargs['Env'] = env

                # reset all checkboxes (as they aren't sent when disabled)
                for group in settings:
                    for item in group:
                        if isinstance(item, ItemCheckbox):
                            fullname = '%s.%s' % (group.name, item.name)
                            if fullname not in kwargs:
                                kwargs[fullname] = 'off'

                for k,v in kwargs.items():
                    # ignore database password unless something was entered
                    if k == 'Database.Password' and len(v) == 0:
                        continue

                    try:
                        settings[k] = v
                    except ValueError as e:
                        settings.item(k).message = str(e)
                        error = True

            if not error:
                settings.persist()
                raise cherrypy.HTTPRedirect('/maintenance')

        return template.render(settings=settings)

    @cherrypy.expose()
    def reset(self):
        daemon.reset()
        raise cherrypy.HTTPRedirect('/maintenance')

    @cherrypy.expose
    def start(self, resolution=None, fullscreen=True):
        daemon.start(resolution, fullscreen)
        raise cherrypy.HTTPRedirect('/maintenance')

    @cherrypy.expose
    def stop(self):
        daemon.stop()
        raise cherrypy.HTTPRedirect('/maintenance')

    @cherrypy.expose
    def reload(self):
        daemon.reload()
        raise cherrypy.HTTPRedirect('/maintenance')

    @cherrypy.expose
    def coredump(selfs):
        cherrypy.response.headers['Content-Type'] = 'application/octet-stream'
        cherrypy.response.headers['Content-Disposition'] = 'attachment; filename=core'

        settings = Settings()
        cwd = settings['Path.BasePath']

        return open(os.path.join(cwd, 'core'))

    @cherrypy.expose
    def rebuild(self):
        cherrypy.response.headers['Content-Type'] = 'text/html'
        import threading
        class Progress(threading.Thread):
            def __init__(self):
                threading.Thread.__init__(self)
                self._sem = threading.Semaphore(value=0)
                self._content = []

            def push(self, x):
                self._content.append(x)
                self._sem.release()

            def __iter__(self):
                return self

            def next(self):
                self._sem.acquire()
                value = self._content.pop()
                if value == None:
                    raise StopIteration()

                return str(value)

            def run(self):
                try:
                    browser = browser_factory.from_settings(Settings())
                    browser.connect()

                    event.trigger('maintenance.rebuild', self.push)
                    self.push('Finished, <a href="/maintenance">go back</a>.')
                    self.push(None)
                except:
                    traceback.print_exc()

        progress = Progress()
        progress.start()

        return progress
    rebuild._cp_config = {'response.stream': True, 'tools.gzip.on' : False}
