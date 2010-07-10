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
	
	def rename(self, c, name):
		c.execute("""
			UPDATE
				queue
			SET
				name = :name
			WHERE
				id = :id
		""", dict(id=self.id, name=name))
		self.name = name

def all(c):
	return [Queue(c, **x) for x in c.execute("""
		SELECT
			id,
			name
		FROM
			queue
	""").fetchall()]

def from_id(c, id):
	row = c.execute("""
		SELECT
			id,
			name
		FROM
			queue
		WHERE
			id = :id
		LIMIT 1
	""", dict(id=id)).fetchone()
	
	if row is None:
		return None
	
	return Queue(c, **row) 

def add(c, name):
	c.execute("""
		INSERT INTO queue (
			name
		) VALUES (
			:name
		)
	""", dict(name=name))

def delete(c, id):
	if id <= 0:
		return False
	
	c.execute("""
		UPDATE
			slide
		SET
			queue_id = 0
		WHERE
			queue_id = :id
	""", dict(id=id))
	c.execute("""
		DELETE FROM
			queue
		WHERE
			id = :id
	""", dict(id=id))
	
	return True
	