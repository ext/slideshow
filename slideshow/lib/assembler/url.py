#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
from slideshow.lib.resolution import Resolution
from slideshow.settings import Settings
import subprocess, pipes
import os, sys, traceback
from os.path import normpath, join, dirname

preview_width = 200

class UrlAssembler(Assembler):
    def is_editable(self):
        return False

    def assemble(self, slide, url, cache, **kwargs):
        return {
            'url': url,
            'cache': cache == '1',
        }

    @staticmethod
    def _phantomjs_path():
        return normpath(join(dirname(__file__), '../../node_modules/phantomjs-prebuilt/bin/phantomjs'))

    @staticmethod
    def _rasterize_path():
        return normpath(join(dirname(__file__), 'url.js'))

    def raster_is_valid(self, size, params):
        if params['cache']: return True

        # hack: dont update preview (causes infinite recursion is trying to render the slideshow overview page)
        if size.w == preview_width: return True

        return False

    def rasterize(self, slide, size, params):
        print repr(params)
        args = [
            self._phantomjs_path(),
            self._rasterize_path(),
            params['url'],
            slide.raster_path(size),
            '%dpx*%dpx' % tuple(iter(size))
        ]

        print os.getcwd()
        print repr(args)

        try:
            retcode = subprocess.call(args)
        except OSError, e:
            raise RuntimeError, 'Failed to run `%s`: %s' % (' '.join([pipes.quote(x) for x in args]), e)
        except:
            traceback.print_exc()
            print >> sys.stderr, 'When running', args
            return

        if retcode != 0:
            raise ValueError, 'failed to resample %s' % (slide.raster_path(size))

    def default_size(self, slide, params, width=None):
        resolution = Settings().resolution()
        if width:
            return resolution.scale(width=width)
        else:
            return resolution

    def title(self):
        return 'Url'
