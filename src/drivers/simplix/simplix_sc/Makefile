##############################################################################
#
#    file                 : Makefile
#    created              : 05 Dez 2008
#    last changed         : 11 Jan 2009
#    copyright            : (C) 2009 Wolf-Dieter Beelitz
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = simplix_sc
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
SOURCES     = ../src/unitmain.cpp \
		../src/unitcarparam.cpp \
		../src/unitcharacteristic.cpp \
		../src/unitclothoid.cpp \
		../src/unitcollision.cpp \
		../src/unitcommon.cpp \
		../src/unitcommondata.cpp \
		../src/unitcubic.cpp \
		../src/unitcubicspline.cpp \
		../src/unitdriver.cpp \
		../src/unitfixcarparam.cpp \
		../src/unitlane.cpp \
		../src/unitlanepoint.cpp \
		../src/unitlinalg.cpp \
		../src/unitlinreg.cpp \
		../src/unitopponent.cpp \
		../src/unitparabel.cpp \
		../src/unitparam.cpp \
		../src/unitpidctrl.cpp \
		../src/unitpit.cpp \
		../src/unitpitparam.cpp \
		../src/unitsection.cpp \
		../src/unitstrategy.cpp \
		../src/unitsysfoo.cpp \
		../src/unitteammanager.cpp \
		../src/unittmpcarparam.cpp \
		../src/unittrack.cpp \
		../src/unitvec2d.cpp \
		../src/unitvec3d.cpp
SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml default.xml logo.rgb readme.txt
SHIPSUBDIRS = 0 1 2 3 4 5 6 7 8 9 10 11 12 13 sc-996 sc-ciclon-300 sc-f360 sc-gt4 sc-lynx-220 sc-nsx tracks

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_sc_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_sc_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${SPEED_DREAMS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
