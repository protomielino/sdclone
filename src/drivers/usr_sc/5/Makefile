##############################################################################
#
#    file                 : Makefile
#    created              : 05 Dez 2008
#    last changed         : 05 Dez 2008
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

ROBOT       = usr_sc
SHIPDIR     = drivers/${ROBOT}/5
SHIP        = 36GP-autounion.png skill.xml
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_trb1a_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_trb1a_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

