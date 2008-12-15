##############################################################################
#
#    file                 : Makefile
#    created              : 22 Nov 2008
#    last changed         : 14 Dez 2008
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

ROBOT       = simplix_trb1a
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml default.xml logo.rgb readme.txt
SHIPSUBDIRS = 0 1 2 3 4 5 6 7 8 9 car1-trb1 car2-trb1 car3-trb1 car4-trb1 car5-trb1 tracks

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_trb1a_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_trb1a_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
