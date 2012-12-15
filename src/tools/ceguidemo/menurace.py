#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""Race menu.


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

# Race menu
class MenuRace(MenuStandard):

	def __init__(self):

		MenuStandard.__init__(self, MenuCredits, MenuOptions) #, MenuProfiles, )
	
	# Initialize
	def initialize(self):

		name = "MenuRace"

		# Use layout if specified.
		if TheConfig.useLayouts:
			
			window = MenuStandard.initialize(self, name=name, title="Race", layout="menurace")
			
		else:
			
			# If no layout specified, go on building up the menu through code.
			window = MenuStandard.initialize(self, name=name, title="Race", background="SplashRace")

			# Specific to this menu.
			btnStart = PyCEGUI.WindowManager.getSingleton().createWindow("CEGUIDemo/Button", name + "/BtnStart")
			btnStart.setText("Start")
			btnStart.setTooltipText("Start to the main menu")
			btnStart.setXPosition(PyCEGUI.UDim(0.75, 0.0))
			btnStart.setYPosition(PyCEGUI.UDim(0.9, 0.0))
			btnStart.setWidth(PyCEGUI.UDim(0.15, 0.0))
			btnStart.setHeight(PyCEGUI.UDim(0.05, 0.0))
			btnStart.setProperty("Font", "MenuMedium")

			window.addChildWindow(btnStart)

			btnBack = PyCEGUI.WindowManager.getSingleton().createWindow("CEGUIDemo/Button", name + "/BtnBack")
			btnBack.setText("Back")
			btnBack.setTooltipText("Back to the main menu")
			btnBack.setXPosition(PyCEGUI.UDim(0.1, 0.0))
			btnBack.setYPosition(PyCEGUI.UDim(0.9, 0.0))
			btnBack.setWidth(PyCEGUI.UDim(0.15, 0.0))
			btnBack.setHeight(PyCEGUI.UDim(0.05, 0.0))
			btnBack.setProperty("Font", "MenuMedium")

			window.addChildWindow(btnBack)

		# Retrieve the window descendants created here.
		self.btnBack = window.getChild(name + "/BtnBack")
		self.btnStart = window.getChild(name + "/BtnStart")

		# Complete widget initialization (whatever creation mode : code or .layout).

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
