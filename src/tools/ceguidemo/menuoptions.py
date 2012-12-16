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
		
		# No code written for this menu : use mandatory layout.
		window = Menu.initialize(self, name=name, title="Options", layout="menuoptions")

		# Retrieve window descendants created here.
		self.btnAccept = window.getChild(name + "/BtnAccept")
		self.btnCancel = window.getChild(name + "/BtnCancel")
		self.cbxWinSize = window.getChild(name + "/CbxWindowSize")
		
		# Complete widget initialization.
		self.cbxWinSizeItems = []
		cbxItem = PyCEGUI.ListboxTextItem(" 800 x  512")
		self.cbxWinSize.addItem(cbxItem)
		self.cbxWinSizeItems.append(cbxItem)
		cbxItem = PyCEGUI.ListboxTextItem("1280 x  800")
		self.cbxWinSize.addItem(cbxItem)
		self.cbxWinSizeItems.append(cbxItem)
		cbxItem = PyCEGUI.ListboxTextItem("1680 x 1050")
		self.cbxWinSize.addItem(cbxItem)
		self.cbxWinSizeItems.append(cbxItem)

		# TODO.

		return window

	# connectHandlers
	# - Wrapper method to define the subscription/listener relationships.
	# - If there are a lot, it may behoove the coder to encapsulate them in methods, then call those methods here.
	def connectHandlers(self):

		# Inherited connections.
		Menu.connectHandlers(self)

		# Specific connections.
		self.btnCancel.subscribeEvent(PyCEGUI.PushButton.EventClicked, self, "onCancelButtonClicked")
		self.btnAccept.subscribeEvent(PyCEGUI.PushButton.EventClicked, self, "onAcceptButtonClicked")

	def onCancelButtonClicked(self, args):

		print("onCancelButtonClicked")
		self.back()

	def onAcceptButtonClicked(self, args):

		print("onAcceptButtonClicked")
		self.back()

