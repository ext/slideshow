#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy, urllib, os
import traceback
import json
import itertools
from slideshow.lib import assembler, queue, slide, template
from slideshow.lib.resolution import Resolution
from slideshow.settings import Settings
import slideshow.daemon as daemon
from cherrypy.lib.static import serve_file

class Ajax(object):
    @cherrypy.expose
    def move(self, queue, slides):
        queue = queue[6:] # remove 'queue_'-prefix
        slides = [x[6:] for x in slides.split(',')] # remove 'slide_'-prefix
        c = cherrypy.thread_data.db

        # prepare slides
        slides = [dict(queue=queue, id=id, order=n) for (n,id) in enumerate(slides)]

        c.executemany("""
            UPDATE
                slide
            SET
                queue_id = :queue,
                sortorder = :order
            WHERE
                id = :id
        """, slides)
        cherrypy.thread_data.db.commit()
        daemon.reload()

        return None

    @cherrypy.expose
    def delete(self, id):
        try:
            slide.delete(cherrypy.thread_data.db, id)
            return json.dumps({'success': True})
        except Exception, e:
            traceback.print_exc()
            return json.dumps({'success': False, 'message': str(e)})

    @cherrypy.expose
    def activate(self, id):
        try:
            s = slide.activate(cherrypy.thread_data.db, id)
            return json.dumps({'success': True, 'class': s.attributes['class']})
        except Exception, e:
            traceback.print_exc()
            return json.dumps({'success': False, 'message': str(e)})

    @cherrypy.expose
    def deactivate(self, id):
        try:
            s = slide.deactivate(cherrypy.thread_data.db, id)
            return json.dumps({'success': True, 'class': s.attributes['class']})
        except Exception, e:
            traceback.print_exc()
            return json.dumps({'success': False, 'message': str(e)})

class Handler(object):
    ajax = Ajax()

    @cherrypy.expose
    @template.output('slides/view.html')
    def list(self):
        queues = queue.all(cherrypy.thread_data.db)
        settings = Settings()

        id = settings['Runtime.queue']
        active = queues[0]
        for q in queues:
            if q.id == id:
                active = q
                break

        # 'intermediate' queue has id -1 and thus is always first, but we want
        # it placed at the bottom, swap positions.
        tmp = queues.pop(0)
        queues.append(tmp)

        return template.render(queues=queues, active=active)

    @cherrypy.expose
    @cherrypy.tools.response_headers(headers=[('Content-Type', 'image/png')])
    def show(self, id, width=None, height=None):
        s = slide.from_id(cherrypy.thread_data.db, id)

        if width is None:
            size = s.default_size()
        elif height is None:
            size = s.default_size(width)
        else:
            size = Resolution(float(width), float(height))

        s.rasterize(size)

        # @todo static.serve_file
        f = open(s.raster_path(size), 'rb')
        bytes = f.read()
        f.close()

        return bytes

    @cherrypy.expose
    @template.output('slides/play.html', parent='video')
    def play(self, id):
        s = slide.from_id(cherrypy.thread_data.db, id)
        stream = None

        settings = Settings()
        base = settings['Path.BasePath']
        temp = settings['Path.Temp']

        path = s._data['filename']
        root, ext = os.path.splitext(path)
        ext = ext[1:].lower()
        preview = os.path.join(base, temp, 'preview', root + '.flv')

        if os.path.exists(preview):
            stream = '/slides/stream/%s' % id

        return template.render(title=s._data['filename'], stream=stream)

    @cherrypy.expose
    #@cherrypy.tools.response_headers(headers=[('Content-Type', 'application/octet-stream')])
    def stream(self, id):
        s = slide.from_id(cherrypy.thread_data.db, id)

        settings = Settings()
        base = settings['Path.BasePath']
        temp = settings['Path.Temp']

        path = s._data['filename']
        root, ext = os.path.splitext(path)
        ext = ext[1:].lower()
        preview = os.path.join(base, temp, 'preview', root + '.flv')

        return serve_file(preview, content_type=None, disposition='attachment')

    @cherrypy.expose
    @template.output('slides/upload.html', parent='slides')
    def upload(self, **content):
        settings = Settings()

        # get all assembers
        unsorted=assembler.all()

        # manually place image and text first
        sorted = [unsorted.pop('text'), unsorted.pop('image')]
        unsorted = unsorted.items()
        unsorted.sort()
        sorted += [v for k,v in unsorted]

        resolution = settings.resolution().scale(640)
        localdata = dict(itertools.chain(*[x.localdata(content).items() for x in sorted]))

        return template.render(
            assemblers=sorted,
            context='upload', content=content,
            resolution=resolution,
            **localdata)

    @cherrypy.expose
    @template.output('slides/edit.html', parent='slides')
    def edit(self, id, **kwargs):
        settings = Settings()
        s = slide.from_id(cherrypy.thread_data.db, id)

        # @todo using private variable!
        content = s._data.copy()
        content.update(kwargs)
        resolution = settings.resolution().scale(640)

        return template.render(
            slide=s, id=id,
            assembler=assembler.get('text'),
            context="edit", content=content,
            resolution=resolution,
            **assembler.get('text').localdata(kwargs))

    @cherrypy.expose
    def submit(self, id=None, context=None, assembler=None, submit=None, **kwargs):
        if submit == 'preview':
            args = kwargs
            if id != None:
                args['id'] = id

            raise cherrypy.HTTPRedirect('/slides/' + context + '?' + urllib.urlencode(args))

        cherrypy.thread_data.db.transaction()
        try:
            if id == None: # new slide
                s = slide.create(cherrypy.thread_data.db, assembler, kwargs)
            else: #edited slide
                s = slide.edit(cherrypy.thread_data.db, id, assembler, kwargs)
            daemon.reload()
            cherrypy.thread_data.db.commit()
        except:
            cherrypy.thread_data.db.rollback()
            raise
        raise cherrypy.HTTPRedirect('/')

    @cherrypy.expose
    @cherrypy.tools.response_headers(headers=[('Content-Type', 'image/png')])
    def preview(self, theme=None, **kwargs):
        import cStringIO

        dst = cStringIO.StringIO()
        asm = assembler.get('text')

        settings = Settings()
        resolution = settings.resolution()
        kwargs['resolution'] = (resolution.w, resolution.h)

        asm.rasterize(file=dst, size=Resolution(640,640/resolution.aspect()), params=kwargs, theme=theme)

        content = dst.getvalue()
        dst.close()

        return content

    @cherrypy.expose
    def update(self, id, **kwargs):
        db = cherrypy.thread_data.db
        s = slide.from_id(db, id)
        s.update(db, kwargs)

    @cherrypy.expose
    def delete(self, id):
        cherrypy.thread_data.db.transaction()
        try:
            slide.delete(cherrypy.thread_data.db, id)
            cherrypy.thread_data.db.commit()
        except:
            cherrypy.thread_data.db.rollback()
            raise
        raise cherrypy.HTTPRedirect('/')
