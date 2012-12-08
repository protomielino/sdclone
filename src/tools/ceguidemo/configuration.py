#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""Configuration.

Singleton for app. configuration management

"""

# Configuration
class _Configuration(object):

	# Ctor : default settings.
	def __init__(self):

		# If True, load menus from .layout files (otherwise, run dedicated code).
		self.useLayouts = False

# TheConfig : the singleton.
TheConfig = _Configuration()
