#!/usr/bin/env python
# -*- coding: utf-8 -*-

from genshi.template import TemplateLoader
from slideshow.lib import template
import os.path

loader = TemplateLoader(
    os.path.join(os.path.dirname(__file__)),
    auto_reload=True
)

class Assembler:
    name = '' # automatically set in factory initialization

    def is_viewable(self):
        """
        A viewable assembler shows the "Full" action which view original or resized version of the slide.
        """
        return True

    def is_editable(self):
        """
        Tells whenever an assembler is editable or not.
        """
        return False

    def is_playable(self):
        """
        A playable assembler allows to display playable content (e.g. video).
        """
        return False

    def assemble(self, slide, **kwargs):
        """
        Prepares an assembler. It does not rasterize anything.
        """
        raise NotImplementedError

    def rasterize(self, slide, src, size, params):
        """
        Rasterize a slide to the given resolution. It takes the source data
        earlier prepared (using assemble) and creates an image of the given
        resolution.
        """
        raise NotImplementedError

    def default_size(self, slide, params, width=None):
        """
        It returns the default size for this slide. Eg for an image slide it
        returns the resolution of the source image, and for a text slide it
        gives the configured resolution.
        """
        raise NotImplementedError

    def raster_is_valid(**kwargs):
        """
        Determines whenever the cached raster is valid
        """
        return True

    def title(self):
        """
        Get a pretty name of this type of assembler
        """
        raise NotImplementedError

    def localdata(self, content):
        """Define local data that is required by renderer"""
        return {}

    def render(self, content, context):
        """
        Get html representation of the upload/edit form (should include the
        fieldset wrapping the fields)
        """

        kwargs = content.copy()
        kwargs.update(**self.localdata(content))

        func = lambda: template.render(fields=content, assembler=self.name, context=context, **kwargs)
        file = self.name + '.html'
        return template.output(file, doctype=False, loader=loader)(func)()

    def raster_extension(self):
        """
        By default, all raster use PNG as format but some, like the video
        thumbnails need GIF, so overrride this if the raster uses something
        other than PNG.
        """
        return '.png'

import image
import text
import video

_assemblers = {
    'text': text.TextAssembler,
    'image': image.ImageAssembler,
    'video': video.VideoAssembler
}

# setup reverse names
for k,v in _assemblers.items():
    v.name = k

def get(name):
    return _assemblers[name]()

def all():
    return dict([(k,v()) for k,v in _assemblers.items()])
