#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys, subprocess
from slideshow.settings import Settings
from slideshow.lib import browser as browser_factory
import threading, json, re, gamin
from select import select
from fnmatch import fnmatchcase
import time, traceback, pipes

from cherrypy.process import plugins

class PreviewCreator(plugins.SimplePlugin, threading.Thread):
    video_extensions = ['avi', 'mkv', 'wmv', 'mov', 'mp4', 'm4v', 'mpg', 'mpeg', 'img', 'flv']

    def __init__(self, bus):
        plugins.SimplePlugin.__init__(self, bus)
        threading.Thread.__init__(self)
        self._running = False

    def start(self):
        self.bus.log("Starting video preview handler.")
        self._running = True
        threading.Thread.start(self)

    def stop(self):
        self.bus.log("Stopping video preview handler.")
        self._running = False
        self.join()

    def run(self):
        settings = Settings()
        base = settings['Path.BasePath']
        video = settings['Path.Video']
        temp = settings['Path.Temp']

        def callback(path, event):
            root, ext = os.path.splitext(path)
            ext = ext[1:].lower()
            if ext not in PreviewCreator.video_extensions:
                return

            src = os.path.join(base, video, path)
            dst = os.path.join(base, temp, 'preview', root + '.flv')

            if event == gamin.GAMDeleted:
                try:
                    os.remove(dst)
                except:
                    pass

            if event in [gamin.GAMChanged, gamin.GAMExists]:
                refresh = False

                st_src = os.stat(src)

                try:
                    st_dst = os.stat(dst)
                    refresh = st_src.st_mtime > st_dst.st_mtime
                except OSError:
                    refresh = True

                if not refresh:
                    return

                try:
                    os.mkdir(os.path.join(base, temp, 'preview'))
                except:
                    pass

                try:
                    def prog(x):
                        print 'Generating preview for %s: %3d%%\r' % (path, x),
                        sys.stdout.flush()
                    self.generate(src, dst, progress_fn=prog)
                except:
                    traceback.print_exc()
                    os.remove(dst)

        path = os.path.join(base, video)
        mon = gamin.WatchMonitor()
        mon.watch_directory(path, callback)
        while self._running:
            time.sleep(1)
            if mon.event_pending() > 0:
                mon.handle_one_event()
                mon.handle_events()
        mon.stop_watch(path)

    def generate(self, src, dst, progress_fn=None, finished_fn=None):
        args = [
            'ffmpeg',
            '-i', src,
            '-ar', '11025',
            '-y',
            dst
        ]

        try:
            proc = subprocess.Popen(args, stderr=subprocess.PIPE)
        except OSError, e:
            raise RuntimeError, 'Failed to run `%s`: %s' % (' '.join([pipes.quote(x) for x in args]), e)

        buffer = ''
        duration = 0.0
        progress = 0.0
        p_duration = re.compile('.*Duration: ([0-9]+):([0-9]+):([0-9\.]+),.*')
        p_frame = re.compile('.*frame.*time=([0-9.]+) .*')

        while True:
            if proc.poll() != None:
                break

            if not self._running:
                raise RuntimeError, 'aborted'

            (rlist, _, _) = select([proc.stderr], [], [], 1.0)

            if proc.stderr in rlist:
                ch = proc.stderr.read(1)
                if ch in ["\n", "\r"]:
                    if progress_fn:
                        match = p_frame.match(buffer)
                        if match:
                            progress = float(match.group(1))
                            progress_fn(int(progress / duration * 100))

                    match = p_duration.match(buffer)
                    if match:
                        h,m,s = match.groups()
                        duration = int(h) * 3600 + int(m) * 60 + float(s)

                    buffer = ''
                else:
                    buffer += ch

        if finished_fn:
            finished_fn(True)
