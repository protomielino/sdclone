#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""Track selection menu.


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
from menucarselect import MenuCarSelect

# Track selection menu
class MenuTrackSelect(MenuStandard):

	def __init__(self):

		MenuStandard.__init__(self, MenuCredits, MenuOptions) #, MenuProfiles, )

		self.menuCarSelect = None
	
	# Initialize
	def initialize(self):

		name = "MenuTrackSelect"
		
		# No code written for this menu : use mandatory layout.
		window = MenuStandard.initialize(self, name=name, title="Select a track", layout="menutrackselect")

		# Retrieve the window descendants created here.
		self.btnBack = window.getChild(name + "/BtnBack")
		self.btnNext = window.getChild(name + "/BtnNext")
		self.cbxCat  = window.getChild(name + "/CbxCategory")

		# Complete widget initialization.
		self.cbxCatItems = []
		for cat in ("Grand Prix", "Road", "Dirt"):
			cbxItem = PyCEGUI.ListboxTextItem(cat)
			cbxItem.setSelectionBrushImage("CEGUIDemo", "ComboboxSelectionBrush")
			cbxItem.setSelectionColours(0xFF3FFFEE)
			self.cbxCat.addItem(cbxItem)
			self.cbxCat.setText(cbxItem.getText())
			self.cbxCatItems.append(cbxItem) # Avoid its being GC'd at return !
			if cat.startswith("Grand"):
				self.cbxCat.setText(cbxItem.getText())

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
		self.btnNext.subscribeEvent(PyCEGUI.PushButton.EventClicked, self, "onNextButtonClicked")

	# Handler: buttonClicked
	def onNextButtonClicked(self, args):

		if not self.menuCarSelect:
			self.menuCarSelect = MenuCarSelect()
			self.menuCarSelect.initialize()
			self.menuCarSelect.setup()

		self.menuCarSelect.activate(previous=self)

	def onBackButtonClicked(self, args):

		self.back()
