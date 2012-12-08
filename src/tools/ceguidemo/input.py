#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# input.py


"""Input functionality.

This class sets up input processing by creating handlers for GLUT that inject
input into PyCEGUI.

"""

# Import: std
import sys

# Import: OpenGL
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

# Import: PyCEGUI
import PyCEGUI

# Import: User
from constants import *
from errors import InitializationError


# Input
class Input(object):

	# Initialize: Handlers
	def initializeHandlers(self):
	
		glutKeyboardFunc(self.handlerKeyDown)
		glutMouseFunc(self.handlerMouseButton)

		# The difference between these two is that the passive one is called when there is
		# mouse motion while no buttons are pressed, and the other is called when there
		# is mouse motion while buttons are pressed. See PyOpenGL documentation.
		glutMotionFunc(self.handlerMouseMotion)
		glutPassiveMotionFunc(self.handlerMouseMotion)

	# Initialize
	def initialize(self):
	
		try:
			self.initializeHandlers()
		except Exception, msg:
			raise InitializationError(msg)

	# Handler: Key Down
	# - `ord` is a built-in Python function.
	def handlerKeyDown(self, key, x, y):

		PyCEGUI.System.getSingleton().injectChar(ord(key))

	# Handler: Mouse Button
	def handlerMouseButton(self, button, state, x, y):

		if button == GLUT_LEFT_BUTTON:
			if state == GLUT_UP:
				PyCEGUI.System.getSingleton().injectMouseButtonUp(PyCEGUI.LeftButton)
			else:
				PyCEGUI.System.getSingleton().injectMouseButtonDown(PyCEGUI.LeftButton)

		# A thought is to turn this into an `else` clause; however, this implies that any
		# button besides the left is interpreted as the right button - this seems undesirable
		# for any mouse with more than two buttons.
		elif button == GLUT_RIGHT_BUTTON:
			if state == GLUT_UP:
				PyCEGUI.System.getSingleton().injectMouseButtonUp(PyCEGUI.RightButton)
			else:
				PyCEGUI.System.getSingleton().injectMouseButtonDown(PyCEGUI.RightButton)

		# An `else` clause could also go here to perform some arbitrary action on unhandled
		# mouse input; this is left as an exercise for the reader. Instead, we just implicitly
		# ignore it.

	# Handler: Mouse Motion
	# - This might seem arbitrary, but in fact this is required or else the position of the mouse
	# will never be updated inside the window.
	def handlerMouseMotion(self, x, y):

		PyCEGUI.System.getSingleton().injectMousePosition(x, y)
