##############################################################################
#
#    file                 : Makefile
#    created              : Wed Jan 8 18:31:16 CET 2003
#    copyright            : (C) 2002-2004 Bernhard Wymann
#    email                : berniw@bluewin.ch
#    version              : $Id: Makefile,v 1.6 2004/09/23 14:04:17 berniw Exp $
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = hymie_36GP
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
SOURCES     = ${ROBOT}.cpp \
		Array.cpp AveragedData.cpp Avoidance.cpp BendAnalysis.cpp CarModel.cpp \
		ClothoidPath.cpp Cubic.cpp CubicSpline.cpp GenericAvoidance.cpp \
		Hysteresis.cpp InsideLineAvoidance.cpp LearnedGraph.cpp \
		LinearAttractor.cpp LinearRegression.cpp LinePath.cpp MyRobot.cpp \
		MyTrack.cpp \
		Opponent.cpp OptimisedPath.cpp OutsideLineAvoidance.cpp ParametricCubic.cpp \
		Path.cpp PathRecord.cpp Pattern.cpp PidController.cpp PitControl.cpp \
		PitPath.cpp PtInfo.cpp Quadratic.cpp Seg.cpp Shared.cpp \
		Span.cpp TeamInfo.cpp Utils.cpp Vec2d.cpp Vec3d.cpp

SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml logo.rgb
SHIPSUBDIRS = 1 2 3 4 5 6 7 8 9 10 11 12 cars

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-hymie_36GP_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-hymie_36GP_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
