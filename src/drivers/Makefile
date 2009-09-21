##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:08:21 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.4 2002/06/30 14:11:14 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

# Uncomment to build just every present robot
#SUBDIRS = $(shell find * -maxdepth 0 -type d -print)

# Build just every present robot but educational "bt" and "K1999"
SUBDIRS = $(shell find * -maxdepth 0 -type d -not -name 'bt' -not -name 'K1999' -not -name '.svn' -print)

SHIPSUBDIRS = $(SUBDIRS)

PKGSUBDIRS	= $(SUBDIRS)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCSNG_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


