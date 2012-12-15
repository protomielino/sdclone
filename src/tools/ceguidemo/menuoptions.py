#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""Options menu.


"""

# Import: std
import sys

# Import: PyCEGUI
import PyCEGUI

# Import: Configuration
from configuration import TheConfig

# Import: Menu
from menu import Menu

# Main menu
class MenuOptions(Menu):

	def __init__(self):

		Menu.__init__(self)
	
	# Initialize
	def initialize(self):

		name = "MenuOptions"

		# Use layout if specified.
		if TheConfig.useLayouts:
			
			window = Menu.initialize(self, name=name, title="Options", layout="menuoptions")
			
		else:
			
			# If no layout specified, go on building up the menu through code.
			window = Menu.initialize(self, name=name, title="Options", background="SplashOptions")

			# Specific to this menu.
			winMgr = PyCEGUI.WindowManager.getSingleton()

			btnBack = PyCEGUI.WindowManager.getSingleton().createWindow("CEGUIDemo/Button", name + "/BtnBack")
			btnBack.setText("Back")
			btnBack.setTooltipText("Back to the main menu")
			btnBack.setXPosition(PyCEGUI.UDim(0.43, 0.0))
			btnBack.setYPosition(PyCEGUI.UDim(0.9, 0.0))
			btnBack.setWidth(PyCEGUI.UDim(0.15, 0.0))
			btnBack.setHeight(PyCEGUI.UDim(0.05, 0.0))
			btnBack.setProperty("Font", "MenuMedium")

			window.addChildWindow(btnBack)

		# Retrieve window descendants created here.
		self.btnBack = window.getChild(name + "/BtnBack")
		
		# Complete widget initialization (whatever creation mode : code or .layout).
		# TODO.

		return window

	# connectHandlers
	# - Wrapper method to define the subscription/listener relationships.
	# - If there are a lot, it may behoove the coder to encapsulate them in methods, then call those methods here.
	def connectHandlers(self):

		# Inherited connections.
		Menu.connectHandlers(self)

		# Specific connections.
		self.btnBack.subscribeEvent(PyCEGUI.PushButton.EventClicked, self, "onBackButtonClicked")

	def onBackButtonClicked(self, args):

		print("onBackButtonClicked")
		self.back()

