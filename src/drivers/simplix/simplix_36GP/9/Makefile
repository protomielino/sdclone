##############################################################################
#
#    file                 : Makefile
#    created              : 05 Dez 2008
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

ROBOT       = simplix_36GP
SHIPDIR     = drivers/${ROBOT}/9
SHIP        = 36GP-bugatti.png
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_36GP_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_36GP_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

