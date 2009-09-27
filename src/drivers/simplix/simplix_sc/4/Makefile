##############################################################################
#
#    file                 : Makefile
#    created              : 05 Dez 2008
#    last changed         : 10 Jan 2009
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
SHIPDIR     = drivers/${ROBOT}/4
SHIP        = sc-f360.png
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_sc_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_sc_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${SPEED_DREAMS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

