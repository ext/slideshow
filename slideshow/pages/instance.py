#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cherrypy, json
from slideshow.settings import Settings
import slideshow.daemon as daemon
import slideshow.event as event

class Handler(object):
	@cherrypy.expose
	def settings(self, name, format='raw'):
		if format == 'raw':
			cherrypy.response.headers['Content-Type'] = 'application/json'
		elif format == 'text':
			cherrypy.response.headers['Content-Type'] = 'text/plain'
		
		settings = Settings()
		
		return json.dumps({
			'Queue': settings['Runtime.queue'],
			'Transition': settings['Appearance.Transition'],
			'TransitionTime': settings['Appearance.TransitionTime'],
			'SwitchTime': settings['Appearance.SwitchTime'],
		}, indent=4)

	@cherrypy.expose
	def next(self, id, name, version, format='raw'):
		if format == 'raw':
			cherrypy.response.headers['Content-Type'] = 'application/json'
		elif format == 'text':
			cherrypy.response.headers['Content-Type'] = 'text/plain'
		
		settings = Settings()
		c = cherrypy.thread_data.db
		
		row = self._get_next_slide(queue=settings['Runtime.queue'], context=id)
        
		if not row:
			loop = c.execute("SELECT loop FROM queue WHERE id = :queue LIMIT 1", dict(queue=settings['Runtime.queue'])).fetchone()['loop']
			row = {
				'assembler': None,
				'id': None,
				'sortorder': None
			}
		
			if loop:
				tmp = self._get_next_slide(queue=settings['Runtime.queue'], context=-1)
				row = tmp and tmp or row
		
		return json.dumps({
			'version': 1,
			'assembler': row['assembler'],
			'filename': None,
			'slide-id': row['id'],
			'context': row['sortorder'],
		}, indent=4)

	def _get_next_slide(self, **kwargs):
		c = cherrypy.thread_data.db
		return c.execute("""
		    SELECT
		        id,
		        path,
		        sortorder,
		        queue_id,
		        assembler
		    FROM
		        slide
		    WHERE
		        queue_id = -1
		UNION
		    SELECT
		        id,
		        path,
		        sortorder,
		        queue_id,
		        assembler
		    FROM
		        slide
		    WHERE
		        queue_id = :queue AND
		        sortorder > :context
		ORDER BY
		    queue_id,
		    sortorder
		LIMIT 1;
		    """, kwargs).fetchone()
