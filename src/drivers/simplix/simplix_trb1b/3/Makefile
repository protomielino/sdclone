##############################################################################
#
#    file                 : Makefile
#    created              : 22 Nov 2008
#    last changed         : 03 Dez 2008
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

ROBOT       = simplix_trb1b
SHIPDIR     = drivers/${ROBOT}/3
SHIP        = logo.rgb car7-trb1.rgb driver.rgb tex-wheel.rgb
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_trb1b_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_trb1b_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

