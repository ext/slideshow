#!/usr/bin/env python
# -*- coding: utf-8 -*-

import socket, struct
import traceback
import cherrypy
from slideshow.event import subscribe, listener
from slideshow.settings import Settings

@listener
class IPBlock(object):
    def __init__(self):
        self.subnet = []

        settings = Settings()
        self.parse_subnet(settings['Access.Subnet'])

        subscribe('config.ipblock_enabled', self.enable)
        subscribe('config.ipblock_changed', self.update)
        cherrypy.config.update({'tools.ipblock.on': settings['Access.Whitelist']})

    def __call__(self):
        remote = self.parse_ip(cherrypy.request.remote.ip)
        for ip, mask in self.subnet:
            if (remote & mask) == ip: return
        raise cherrypy.HTTPError("401 Unauthorized", "IP not in whitelist.")

    def enable(self, value):
        print 'herp derp', value
        cherrypy.config.update({'tools.ipblock.on': value})

    def update(self, value):
        IPBlock.parse_subnet(self, value)

    def parse_subnet(self, data):
        self.subnet = []
        for line in data.split('\n'):
            line = line.strip()
            if len(line) == 0: continue

            try:
                ip = line
                cidr = '32'
                if '/' in line:
                    tmp = line.split('/', 1)
                    ip = tmp[0]
                    cidr = tmp[1]

                mask = self.gen_mask(cidr)
                ip = self.parse_ip(ip) & mask

                self.subnet.append((ip, mask))
            except:
                traceback.print_exc()
        cherrypy.engine.log('Loaded %d subnet(s) to IP whitelist' % len(self.subnet))

    def parse_ip(self, val):
        try:
            _str = socket.inet_pton(socket.AF_INET, val)
        except socket.error:
            raise ValueError, '%s could not be parsed' % val
        return struct.unpack('I', _str)[0]

    def gen_mask(self, cidr):
        cidr = int(cidr)
        return socket.htonl(int('1'*cidr + '0'*(32-cidr), 2))

