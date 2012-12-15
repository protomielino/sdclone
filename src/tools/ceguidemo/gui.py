#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""GUI.

This class is the entry point for the GUI; which is to say that it starts the
main menu and the rest is event driven.

"""

# Import: std
import sys

# Import: PyCEGUI
import PyCEGUI

# Import: User
from errors import InitializationError
from menumain import MenuMain


# GUI
class GUI(object):

	# Initialize: Resources
	def initializeResources(self):
		
		rp = PyCEGUI.System.getSingleton().getResourceProvider()
		rp.setResourceGroupDirectory('schemes', './datafiles/schemes')
		rp.setResourceGroupDirectory('imagesets', './datafiles/imagesets')
		rp.setResourceGroupDirectory('fonts', './datafiles/fonts')
		rp.setResourceGroupDirectory('layouts', './datafiles/layouts')
		rp.setResourceGroupDirectory('looknfeels', './datafiles/looknfeel')
		rp.setResourceGroupDirectory('schemas', './datafiles/xml_schemas')
		PyCEGUI.Imageset.setDefaultResourceGroup('imagesets')
		PyCEGUI.Font.setDefaultResourceGroup('fonts')
		PyCEGUI.Scheme.setDefaultResourceGroup('schemes')
		PyCEGUI.WidgetLookManager.setDefaultResourceGroup('looknfeels')
		PyCEGUI.WindowManager.setDefaultResourceGroup('layouts')
		
		parser = PyCEGUI.System.getSingleton().getXMLParser()
		if parser.isPropertyPresent('SchemaDefaultResourceGroup'):
			parser.setProperty('SchemaDefaultResourceGroup', 'schemas')

	# Initialize: Defaults
	def initializeDefaults(self):
		
		sm = PyCEGUI.SchemeManager.getSingleton()
		sm.create('ceguidemo.scheme')
		PyCEGUI.System.getSingleton().setDefaultMouseCursor('CEGUIDemo', 'MouseArrow')
		PyCEGUI.System.getSingleton().setDefaultTooltip('CEGUIDemo/Tooltip')

	# Initialize
	def initialize(self):
		
		try:
			
			self.initializeResources()
			self.initializeDefaults()
			
		except Exception, msg:
			
			raise InitializationError(msg)

	# Setup
	# - Important: the instance of `Menu` has to be bound to this object; if it is
	# a local variable (read: destroyed when it goes out of scope), exceptions will be raised
	# about the `buttonClicked` method not existing. This is a drawback of the type of setup
	# this example uses, and as a consequence of Python being a garbage collected language.
	def setup(self):
		
		self.menu = MenuMain()
		self.menu.initialize()
		self.menu.setup()
		self.menu.activate()

	# Setup: Interface
	def setupInterface(self):
		
		self.setup()
