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

ROBOT       = usr_trb1
SHIPDIR     = drivers/${ROBOT}/14
SHIP        = car7-trb1.rgb driver.rgb wheel3d.png
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-usr_sc_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-usr_sc_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${SPEED_DREAMS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

