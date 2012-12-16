#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""Car selection menu.


"""

# Import: std
import sys

# Import: PyCEGUI
import PyCEGUI

# Import: Configuration
from configuration import TheConfig

# Import: Menu
from menustandard import MenuStandard
from menucredits import MenuCredits
from menuoptions import MenuOptions

# Car selection menu
class MenuCarSelect(MenuStandard):

	def __init__(self):

		MenuStandard.__init__(self, MenuCredits, MenuOptions) #, MenuProfiles, )
	
	# Initialize
	def initialize(self):

		name = "MenuCarSelect"

		# No code written for this menu : use mandatory layout.
		window = MenuStandard.initialize(self, name=name, title="Select a car", layout="menucarselect")

		# Retrieve the window descendants created here.
		self.btnBack = window.getChild(name + "/BtnBack")
		self.btnStart = window.getChild(name + "/BtnStart")

		# Complete widget initialization.
		# TODO.

		return window
		
	# connectHandlers
	# - Wrapper method to define the subscription/listener relationships.
	# - If there are a lot, it may behoove the coder to encapsulate them in methods, then call those methods here.
	def connectHandlers(self):

		# Inherited connections.
		MenuStandard.connectHandlers(self)

		# Specific connections.
		self.btnBack.subscribeEvent(PyCEGUI.PushButton.EventClicked, self, "onBackButtonClicked")
		self.btnStart.subscribeEvent(PyCEGUI.PushButton.EventClicked, self, "onStartButtonClicked")

	# Handler: buttonClicked
	def onStartButtonClicked(self, args):

		print("onStartButtonClicked")

	def onBackButtonClicked(self, args):

		print("onBackButtonClicked")
		self.back()
