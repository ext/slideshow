#!/usr/bin/env python
# -*- coding: utf-8 -*-

from . import Assembler
from slideshow.lib.resolution import Resolution
from slideshow.settings import Settings
import slideshow
import array, cairo, pango, pangocairo, json, re
import os
import xml
from xml.dom import minidom
from htmlcolor import Parser as ColorParser
from os.path import join, dirname, basename, abspath, realpath
import htmlcolor
import urllib
import traceback

def decode_position(str, size):
    """
    Decode a position string to a tuple with the element sizes
    Accepts either absolute positions or as a percentage of the full size.
    Negative positions are subtracted from the size.

    '-50 50%' -> (750, 300) (given that size is (800,600))
    """

    if str == '':
        return None

    # parse an element
    # x is element
    # s is the total dimension
    def f(x,s):
        if x[-1] == '%':
            v = float(x[:-1]) / 100.0 * s
        else:
            v = float(x)

        if v < 0.0:
            v = s + v

        return v

    # split into pieces 'x y' -> [x, y]
    p = str.split(' ')

    # parse each element
    p = [f(x,s) for x,s in zip(p, size)]

    # create tuple
    return tuple(p)

color_parser = ColorParser(components=4, factory=htmlcolor.FloatFactory)
decode_color = color_parser.parse

class Item:
    def __init__(self, name, title):
        self.name = name
        self.title = title

    def render(self, content):
        raise NotImplementedError

    def raster(self, cr, size, realsize, scale, content):
        raise NotImplementedError

    @staticmethod
    def _get(element, key, default=None, parser=None, *args, **kwargs):
        value = element.getAttribute(key) or default

        if parser is not None:
            value = parser(value, *args, **kwargs)

        return value

    @staticmethod
    def factory(element):
        lut = {
            'text': Label,
            'textarea': TextArea
        }

        func = lut.get(element.tagName, None)
        if func is None:
            return None

        return func(element)

class TextArea(Item):
    def __init__(self, element):
        _ = element.getAttribute
        Item.__init__(self, _('name'), _('title'))

        self._font      = Item._get(element, 'font', 'Sans')
        self._fontsize  = Item._get(element, 'size', '36.0', float)
        self._color     = Item._get(element, 'color', '#000', decode_color)
        self._position  = Item._get(element, 'position', '0 0')
        self._boxsize   = Item._get(element, 'boxsize')
        self._alignment = Item._get(element, 'align', 'left')

    def render(self, content):
        return ('<label for="slide-{name}">{title}</label><br/>' +
                '<textarea id="slide-{name}" name="{name}" cols="80" rows="15">{content}</textarea><br/>') \
                .format(name=self.name, title=self.title, content=content)

    def raster(self, cr, size, realsize, scale, content):
        font = self._font
        fontsize = self._fontsize * scale
        r,g,b,a = self._color
        x,y = decode_position(self._position, realsize)
        w,h = decode_position(self._boxsize, realsize) or size
        alignment = self._alignment

        cr.set_source_rgba(r,g,b,a)
        cr.move_to(x-w/2.0, y-h/2.0)

        ctx = pangocairo.CairoContext(cr)
        layout = ctx.create_layout()
        layout.set_font_description(pango.FontDescription('%s %f' % (font, fontsize)))
        layout.set_width(int(w * pango.SCALE))

        if alignment == 'center':
            layout.set_alignment(pango.ALIGN_CENTER)
        elif alignment == 'right':
            layout.set_alignment(pango.ALIGN_RIGHT)
        elif alignment == 'justify':
            layout.set_justify(True)

        layout.set_markup(content);
        ctx.show_layout(layout)

class Label(Item):
    def __init__(self, element):
        _ = element.getAttribute
        Item.__init__(self, _('name'), _('title'))

        self._font      = Item._get(element, 'font', 'Sans')
        self._fontsize  = Item._get(element, 'size', '36.0', float)
        self._color     = Item._get(element, 'color', '#000', decode_color)
        self._position  = Item._get(element, 'position', '0 0')
        self._alignment = Item._get(element, 'align', 'left')

    def render(self, content):
        return ('<label for="slide-{name}">{title}</label><br/>' +
                '<input type="text" id="slide-{name}" name="{name}" size="70" value="{content}" /><br/>') \
                .format(name=self.name, title=self.title, content=content)

    def raster(self, cr, size, realsize, scale, content):
        font = self._font
        fontsize = self._fontsize * scale
        r,g,b,a = self._color
        x,y = decode_position(self._position, realsize)
        alignment = self._alignment

        cr.select_font_face (font, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)
        cr.set_font_size(fontsize)
        cr.set_source_rgba(r,g,b,a)

        extents = cr.text_extents(content)

        offset = 0
        if alignment == "center":
            x -= extents[4] / 2
        elif alignment == "right":
            x -= extents[4]

        cr.move_to(x, y)
        cr.show_text(content)

class Theme:
    def __init__(self, filename):
        settings = Settings()

        search_path = [
            join(settings['Path.BasePath'], settings['Path.Theme']),
            join(dirname(slideshow.__file__), 'themes'),
            '.']

        if filename[0] == '/': # absolute path
            self.fullpath = filename
            self.basename = basename(filename)
            self.dirname = dirname(filename)
            if not os.path.exists(filename):
                raise RuntimeError, 'Could not find theme-file "%s"' % filename
        else: # relative path
            for path in search_path:
                test = abspath(join(path, filename))
                if os.path.exists(test):
                    self.fullpath = realpath(join(path, filename))
                    self.basename = filename
                    self.dirname = realpath(path)
                    fullpath = test
                    break
            else:
                raise RuntimeError, 'Could not find theme-file "%s"' % filename

        self._doc = minidom.parse(self.fullpath)
        self._template = self._doc.getElementsByTagName('template')[0]
        self._items = [Item.factory(x) for x in self._template.childNodes if isinstance(x, minidom.Element)]

    def rasterize(self, dst, size, params):
        resolution = params['resolution']
        realsize = resolution.fit(size)

        # scale constant
        scale = float(realsize.w) / resolution.w

        data = array.array('c', chr(0) * int(size.w) * int(size.h) * 4)
        surface = cairo.ImageSurface.create_for_data(data, cairo.FORMAT_ARGB32, int(size.w), int(size.h), int(size.w)*4)
        cr = cairo.Context(surface)

        cr.translate((size.w-realsize.w)*0.5, (size.h-realsize.h)*0.5)

        cr.save()
        cr.set_source_rgba(0.5, 0.0 , 0.5, 0.3)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()

        cr.set_source_rgba(0, 0, 0, 1)
        cr.rectangle(0, 0, realsize.w, realsize.h)
        cr.fill()
        cr.restore()

        background = self._template.getAttribute('background')
        if background:
            background = background.strip()

            cr.save()

            if background[:3] == 'url':
                url = background[4:-1] # extract filename from "url(filename)"

                # url is relative to theme-directory unless it begins with /
                if not url[0] == '/':
                    settings = Settings()
                    url = os.path.join(settings['Path.BasePath'], settings['Path.Theme'], url)

                try:
                    image = cairo.ImageSurface.create_from_png(url)
                except Exception, e:
                    raise OSError, 'Failed to load background "%s": %s' % (url, e)

                w = image.get_width()
                h = image.get_height()

                cr.scale(realsize.w/w, realsize.h/h)
                cr.set_source_surface(image, 0, 0)
                cr.paint()
            else:
                try:
                    cr.set_source_rgba(*decode_color(background))
                except:
                    traceback.print_exc()
                    cr.set_source_rgba(0,0,0,1)
                cr.rectangle(0, 0, realsize.w, realsize.h)
                cr.fill()

            cr.restore()

        for item in self.items():
            cr.save()
            try:
                item.raster(cr, size, realsize, scale, params[item.name])
            finally:
                cr.restore()

        # write_to_png doesn't accept unicode string, yet
        if isinstance(dst, basestring):
            dst = str(dst)

        surface.write_to_png(dst)

    def items(self):
        return self._items

class TextAssembler(Assembler):
    def default_size(self, slide, params, width=None):
        resolution = Settings().resolution()
        if width:
            return resolution.scale(width=width)
        else:
            return resolution

    def is_editable(self):
        return True

    def assemble(self, slide, **kwargs):
        kwargs = kwargs.copy()
        kwargs['resolution'] = (kwargs['resolution'].w, kwargs['resolution'].h)
        return kwargs

    def rasterize(self, size, params, slide=None, file=None, theme=None):
        settings = Settings()

        params = params.copy()
        params['resolution'] = Resolution(params['resolution'][0], params['resolution'][1])

        dst = slide and slide.raster_path(size) or file

        if theme is None:
            theme = settings['Appearance.Theme']

        template = Theme(theme)
        template.rasterize(dst=dst, size=size, params=params)

    def raster_is_valid(reference, resolution, **kwargs):
        return reference == resolution

    def title(self):
        return 'Text'

    def render(self, content, context):
        settings = Settings()

        default = dict(preview=None)
        if len(content) > 0:
            default['preview'] = urllib.urlencode(content)
        default.update(content)
        default['template'] = Theme(settings['Appearance.Theme'])
        return Assembler.render(self, content=default, context=context)
