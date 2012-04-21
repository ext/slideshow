#!/usr/bin/env python
# -*- coding: utf-8 -*-

from slideshow.lib.slide import Slide
from slideshow.settings import Settings
import slideshow.event as event
import cherrypy

def div_id(id):
    if id >= 0:
        return 'queue_%d' % id
    elif id == -1:
        return 'queue_int'
    else:
        raise ValueError, 'Invalid id for div_id: %d' % id

class Queue:
    def __init__(self, c, id, name, loop):
        self.id = id
        self.div = div_id(id)
        self.name = name
        self.loop = loop == 1
        self.slides = [Slide(queue=self, **x) for x in c.execute("""
            SELECT
                `id`,
                `path`,
                `active`,
                `assembler`,
                `data`
            FROM
                `slide`
            WHERE
                `queue_id` = :queue
            ORDER BY
                `sortorder`
        """, {'queue': id}).fetchall()]
    
    def __len__(self):
        return len(self.slides)
    
    def rename(self, c, name):
        c.execute("""
            UPDATE
                `queue`
            SET
                `name` = :name
            WHERE
                `id` = :id
        """, dict(id=self.id, name=name))
        self.name = name

def all(c):
    return [Queue(c, **x) for x in c.execute("""
        SELECT
            `id`,
            `name`,
            `loop`
        FROM
            `queue`
    """).fetchall()]

def from_id(c, id):
    row = c.execute("""
        SELECT
            `id`,
            `name`,
            `loop`
        FROM
            `queue`
        WHERE
            `id` = :id
        LIMIT 1
    """, dict(id=id)).fetchone()
    
    if row is None:
        return None
    
    return Queue(c, **row) 

def add(c, name):
    
    c.execute("""
        INSERT INTO `queue` (
            `name`
        ) VALUES (
            :name
        )
    """, dict(name=name))
    
    row_id = c.last_row_id()
    n = int(c.execute("SELECT COUNT(*) as `count` FROM `queue`").fetchone()['count'])

    # if no previous queue (except default) existed, make this the active
    if n == 3:
        print 'derp'
        activate(row_id)

def delete(c, id):
    if id <= 0:
        return False
    
    c.execute("""
        UPDATE
            `slide`
        SET
            `queue_id` = 0
        WHERE
            `queue_id` = :id
    """, dict(id=id))
    c.execute("""
        DELETE FROM
            `queue`
        WHERE
            `id` = :id
    """, dict(id=id))
    
    return True

def activate(id):
    settings = Settings()
    
    with settings:
        settings['Runtime.queue'] = id
    
    settings.persist()
    event.trigger('config.queue_changed', id)

def set_loop(id, state):
    c = cherrypy.thread_data.db
    c.execute("""
        UPDATE
            `queue`
        SET
            `loop` = :state
        WHERE
            `id` = :id
    """, dict(id=id, state=state))
    c.commit()
    
    # to force reloading of queue settings
    event.trigger('config.queue_changed', id)
