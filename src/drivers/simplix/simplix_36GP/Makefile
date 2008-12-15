##############################################################################
#
#    file                 : Makefile
#    created              : 22 Nov 2008
#    last changed         : 15 Dec 2008
#    copyright            : (C) 2008 Wolf-Dieter Beelitz
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = simplix_36GP
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
SOURCES     = ../unitmain.cpp \
		../unitcarparam.cpp \
		../unitcharacteristic.cpp \
		../unitclothoid.cpp \
		../unitcollision.cpp \
		../unitcommon.cpp \
		../unitcommondata.cpp \
		../unitcubic.cpp \
		../unitcubicspline.cpp \
		../unitdriver.cpp \
		../unitfixcarparam.cpp \
		../unitlane.cpp \
		../unitlanepoint.cpp \
		../unitlinalg.cpp \
		../unitlinreg.cpp \
		../unitopponent.cpp \
		../unitparabel.cpp \
		../unitparam.cpp \
		../unitpidctrl.cpp \
		../unitpit.cpp \
		../unitpitparam.cpp \
		../unitsection.cpp \
		../unitstrategy.cpp \
		../unitsysfoo.cpp \
		../unitteammanager.cpp \
		../unittmpcarparam.cpp \
		../unittrack.cpp \
		../unitvec2d.cpp \
		../unitvec3d.cpp
SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml default.xml logo.rgb 
SHIPSUBDIRS = 0 1 2 3 4 5 6 7 8 9 tracks 36GP-alfa12c 36GP-alfatipo 36GP-autounion 36GP-bugatti 36GP-maserati 36GP-maserati6C34 36GP-mercedes

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_36GP_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_36GP_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
