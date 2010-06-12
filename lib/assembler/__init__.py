#!/usr/bin/env python
# -*- coding: utf-8 -*-

class Assembler:
	def is_editable(self):
		return False

class ImageAssembler(Assembler):
	def is_editable(self):
		return False

class TextAssembler(Assembler):
	def is_editable(self):
		return True

_assemblers = {
	'text': TextAssembler(),
	'image': ImageAssembler()
}

def get(name):
	return _assemblers[name]
