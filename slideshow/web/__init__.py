#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys
import cherrypy
import sqlite3
import re
import argparse
import traceback
import json
import subprocess
import threading
import time
import cairo

import slideshow
from slideshow.lib import template, browser as browser_factory, slide
from slideshow.pages import instance
from slideshow.pages import slides
from slideshow.pages import maintenance
from slideshow.pages import queue
from slideshow.settings import Settings, ValidationError
from slideshow.daemon import Daemon
from slideshow.video_preview import PreviewCreator
from slideshow.event import subscribe
import slideshow.tools.ipblock

def get_resource_path(*path):
    return os.path.join(os.path.dirname(slideshow.__file__), *path)

def daemon_exists():
    def is_exe(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    for program in ['slideshow-daemon', 'slideshow-transition']:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                break
        else:
            print >> sys.stderr, program, 'could not be found in PATH.'
            return False

    return True

class Command:
    def __init__(self, *args):
        self.cmd = args
        self.process = None
        self.buffer = ''

    def __str__(self):
        return ' '.join(self.cmd)

    def run(self, timeout):
        def target():
            self.process = subprocess.Popen(self.cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            self.buffer += self.process.communicate()[0]

        thread = threading.Thread(target=target)
        thread.start()

        thread.join(timeout)
        if thread.is_alive():
            self.process.terminate()
            thread.join()
        return self.process.returncode, self.buffer

class Root(object):
    slides = slides.Handler()
    maintenance = maintenance.Handler()
    queue = queue.Handler()
    instance = instance.Handler()

    @cherrypy.expose
    def transition(self, name):
        name, ext = os.path.splitext(name)
        if ext != '.gif':
            raise cherrypy._cperror.NotFound()

        settings = Settings()
        filename = 'transition_%s.gif' % name
        dst = os.path.join(settings['Path']['BasePath'], settings['Path']['Temp'], filename)

        age = 60 * 60 * 24 # 1 day
        expires = time.time() + age
        cherrypy.response.headers['Content-Type'] = 'image/gif'
        cherrypy.response.headers['Cache-Control'] = 'public, max-age=%d' % age
        cherrypy.response.headers['Expires'] = time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime(expires))

        cmd = Command('slideshow-transition', '-G', dst, name)
        returncode, output = cmd.run(timeout=15)
        if returncode != 0:
            surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 228, 170)
            ctx = cairo.Context (surface)
            ctx.set_source_rgb(0.7, 0.7, 0.7)
            ctx.fill()
            ctx.select_font_face('Monospace')
            ctx.set_font_size(9)
            ctx.set_source_rgb(0, 0, 0)
            ctx.move_to(1, 10)
            ctx.show_text('Failed to generate preview:')
            ctx.stroke()
            ctx.move_to(1, 20)
            ctx.show_text(output)
            ctx.stroke()

            cherrypy.response.headers['Content-Type'] = "image/png"
            surface.write_to_png(dst)

        def stream():
            with open(dst, 'rb') as fp:
                data = fp.read(4096)
                while len(data) > 0:
                    yield data
                    data = fp.read(4096)
        return stream()
    transition._cp_config = {'response.stream': True}

    @cherrypy.expose
    def index(self):
        raise cherrypy.InternalRedirect('/slides/list')

    @cherrypy.expose
    def logout(self):
        cherrypy.response.headers['www-authenticate'] = 'Digest realm="%s", nonce="foobar", algorithm="MD5", qop="auth"' % cherrypy.request.config['tools.auth_digest.realm']
        raise cherrypy.HTTPError(401, "Unauthorized")

    def logout_error_401(self, status, message, traceback, version):
        return 'You have logged out, please close this window or go back to <a href="/">slideshow</a> (requires relogin).'

def install(dst, config_file):
    from os.path import isdir, join, exists

    # ensure path is a directory
    if not isdir(dst):
        print >> sys.stderr, 'Must point to a directory, not', dst
        return 1

    for x in ['image', 'video', 'tmp', 'theme']:
        os.mkdir(join(dst, x))

    if not config_file:
        config_file = join(dst, 'settings.conf')

    # check if config already exists
    if exists(config_file):
        print >> sys.stderr, 'Configuration already exists, cannot install a new one.'
        print >> sys.stderr, 'If you intend to replace the configuration, please remove the old one first, e.g. `rm %s\'' % config_file
        return 1

    # put a dummy config, will be overwritten later
    try:
        with open(config_file, 'w') as f:
            f.write("{}\n")
    except IOError:
        print >> sys.stderr, 'Failed to install configuration at', args.config_file
        print >> sys.stderr, 'Ensure that you have write-permission to the specified location.'
        return 1

    # load default settings
    settings = Settings()
    settings.load(get_resource_path('settings.xml'), config_file=None, format_keys=dict(basepath=dst))

    # store the directory we installed to
    with settings:
        settings['Path.BasePath'] = os.path.abspath(dst)

    settings.persist(dst=config_file)
    print >> sys.stderr, 'Configuration installed, restart without --install flag to continue.'
    return 0

def switch(dst, config_file):
    settings = Settings()

    src = None
    try:
        settings.load(get_resource_path('settings.xml'), config_file=config_file, format_keys=dict(basepath=os.path.split(config_file)[0]), errors=['Path.BasePath'])
    except ValidationError, e:
        src = e.value

    # Old basepath still exists, so read it from config
    if src is None:
        src = settings['Path.BasePath']

    # Replace basepath
    with settings:
        settings['Path.BasePath'] = dst
    settings.load(get_resource_path('settings.xml'), config_file=config_file, format_keys=dict(basepath=os.path.split(config_file)[0]), errors=None)

    # load browser
    browser = browser_factory.from_settings(settings)
    browser.connect()

    # update path in  all slides
    [x.switch(browser, dst) for x in slide.all(browser, validate_path=False)]

    # save settings
    settings.persist(dst=config_file)

    return 0

def run():
    # setup argument parser
    parser = argparse.ArgumentParser(description='Slideshow frontend')
    parser.add_argument('-f', '--config-file')
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true')
    parser.add_argument('-q', '--quiet', dest='verbose', action='store_false')
    parser.add_argument('-p', '--port', type=int, default=8000)
    parser.add_argument('--install', default=None)
    parser.add_argument('--switch', default=None)

    # parse args
    args = parser.parse_args()

    try:
        # create new config if we are bootstraping
        if args.install:
            rc = install(args.install, args.config_file)
            sys.exit(rc)

        # verify that a config_file was indeed passed (#101)
        if args.config_file is None:
            print >> sys.stderr, 'Must provide a config-file using the --config-file flag'
            sys.exit(1)

        # verify that config_file exists
        if not os.path.exists(args.config_file):
            print >> sys.stderr, 'Could not open configuration file:', args.config_file
            print >> sys.stderr, 'You can create a new configuration by adding the --install flag or creating the file yourself.'
            print >> sys.stderr, 'Note that the file must be writable by the user running slideshow.'
            sys.exit(1)

        # verify that config_file is writable
        if not os.access(args.config_file, os.W_OK):
            print >> sys.stderr, 'Could not write to configuration file:', args.config_file
            print >> sys.stderr, 'Make sure the user has proper permissions.'
            sys.exit(1)

        # verify slideshow daemon exists in path
        if not daemon_exists():
            # Notice about specific application is written by `daemon_exists`.
            print >> sys.stderr, 'Either install to a location in PATH or adjust PATH to include the directory where it is installed.'
            sys.exit(1)

        # update basepath and references if using --switch
        if args.switch:
            rc = switch(args.switch, args.config_file)
            sys.exit(rc)

        # load slideshow settings
        try:
            settings = Settings()
            settings.load(get_resource_path('settings.xml'), config_file=args.config_file, format_keys=dict(basepath=os.path.split(args.config_file)[0]), errors=['Path.BasePath'])
        except  ValidationError:
            print >> sys.stderr, '"Path.BasePath" does not exist, if you intend to move the datafolder use the --switch flag to update "Path.BasePath" and all references.'
            sys.exit(1)

        # read cherrypy config
        config = settings['cherrypy']

        # cherrypy only accepts str, not unicode
        for k,v in config.items():
            if isinstance(v, basestring):
                config[k] = str(v)

        # load browser
        browser = browser_factory.from_settings(settings)

        # make all worker threads connect to the database
        cherrypy.engine.subscribe('start_thread', browser.connect)

        # load application config
        root = Root()
        application = cherrypy.tree.mount(root, '/')
        application.config.update({
            '/': {
                'tools.staticdir.root': os.path.dirname(os.path.abspath(__file__)),
                'tools.gzip.on': True,
                'tools.encode.on': True,
                'tools.encode.encoding': 'utf8',
                },
            '/static': {
                'tools.staticdir.on': True,
                'tools.staticdir.dir': '../static',
                },
            })

        # add password-protection if available
        passwd = os.path.join(settings['Path.BasePath'], 'users')
        if os.path.exists(passwd):
            application.config['/'].update({
                'tools.auth_digest.on': True,
                'tools.auth_digest.realm': 'slideshow',
                'tools.auth_digest.get_ha1': cherrypy.lib.auth_digest.get_ha1_file_htdigest(passwd),
                'tools.auth_digest.key': None,
            })
            application.config['/instance'] = {
                'tools.auth_digest.on': False,
            }
            application.config['/logout'] = {'error_page.401': root.logout_error_401}

        # add ipblock tool
        cherrypy.tools.ipblock = cherrypy.Tool('on_start_resource', slideshow.tools.ipblock.IPBlock())

        # cherrypy site config
        cherrypy.config.update({
                'sessionFilter.on': True,
                'server.socket_host': "0.0.0.0",
                'server.socket_port': args.port,
            })
        cherrypy.config.update(config)

        # initiate daemon instance
        daemon = Daemon(cherrypy.engine, browser)
        daemon.subscribe()

        # initialize view preview generator
        preview = PreviewCreator(cherrypy.engine)
        preview.subscribe()

        # database migration script
        migration = slideshow.tools.Migration(cherrypy.engine, browser)
        migration.subscribe()

        # start cherrypy
        if hasattr(cherrypy.engine, 'block'):
            # 3.1 syntax
            cherrypy.engine.start()
            cherrypy.engine.block()
        else:
            # 3.0 syntax
            cherrypy.server.quickstart()
            cherrypy.engine.start()

    except Exception, e: # don't leak any exceptions
        if args.verbose:
            traceback.print_exc()
        else:
            print >> sys.stderr, e

        # if this is reached the application crashed
        sys.exit(1)

if __name__ == '__main__':
    run()
