#!/usr/bin/env python
# -*- coding: utf-8 -*-

from slide import Slide

class Queue:
	def __init__(self, c, id, name):
		self.id = id
		self.name = name
		self.slides = [Slide(queue=self, **x) for x in c.execute("""
			SELECT
				id,
				path,
				active,
				assembler,
				data
			FROM
				slide
			WHERE
				queue_id = :queue
			ORDER BY
				sortorder
		""", {'queue': id}).fetchall()]

def all(c):
	return [Queue(c, **x) for x in c.execute("""
		SELECT
			id,
			name
		FROM
			queue
	""").fetchall()]
