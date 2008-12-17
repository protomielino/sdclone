##############################################################################
#
#    file                 : Makefile
#    created              : 07 Dec 2007
#    last changed         : 16 Dec 2008
#    copyright            : (C) 2007-2008 Wolf-Dieter Beelitz
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = simplix
SHIPDIR     = drivers/${ROBOT}/car1-trb1
SHIP        = $(shell find * -maxdepth 0 -type f -print)
SHIPSUBDIRS = $(shell find * -maxdepth 0 -type d -print)

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
