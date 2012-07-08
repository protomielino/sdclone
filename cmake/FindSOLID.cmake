# Locate SOLID libraries (collision detection for solid bodies)
# This module defines
# SOLID_SOLID_LIBRARY, SOLID_BROAD_LIBRARY : name of each lib
# SOLID_LIBRARY : list of lib names
# SOLID_FOUND : if false, do not try to link to SOLID
# SOLID_INCLUDE_DIR : where to find the headers
#
# $SOLID_DIR is an environment variable that would
# correspond to the ./configure --prefix=$SOLID_DIR
# used in building SOLID.
#
# Created by Jean-Philippe Meuret (based on Mart Kelder's FindPLIB.cmake).

IF(NOT APPLE)

  FIND_PATH(SOLID_SOLIDINCLUDE_DIR SOLID/solid.h
    HINTS ENV SOLID_DIR
    PATH_SUFFIXES 
	  include/SOLID include
    PATHS
	  /usr /usr/local
    DOC "Location of SOLID")

ELSE(NOT APPLE)

  FIND_PATH(SOLID_SOLIDINCLUDE_DIR solid.h
    HINTS ENV SOLID_DIR
    PATH_SUFFIXES 
	  Headers include/SOLID include
    PATHS
	  #Additional MacOS Paths
   	  ~/Library/Frameworks/SOLID.framework
  	  /Library/Frameworks/SOLID.framework
 	  /System/Library/Frameworks/SOLID.framework # Tiger

	  /usr /usr/local
    DOC "Location of SOLID")

ENDIF(NOT APPLE)

SET(SOLID_INCLUDE_DIR ${SOLID_SOLIDINCLUDE_DIR} CACHE DOC "Include dir for SOLID")

FIND_LIBRARY(SOLID_SOLID_LIBRARY 
  NAMES solid
  HINTS ENV SOLID_DIR
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS /usr /usr/local)

IF(WIN32)

  FIND_LIBRARY(SOLID_BROAD_LIBRARY
    NAMES broad
    HINTS ENV SOLID_DIR
    PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
    PATHS /usr /usr/local)

ENDIF(WIN32)

IF(SOLID_INCLUDE_DIR AND SOLID_SOLID_LIBRARY AND (NOT WIN32 OR SOLID_BROAD_LIBRARY))
  SET(SOLID_FOUND TRUE)
ENDIF(SOLID_INCLUDE_DIR AND SOLID_SOLID_LIBRARY AND (NOT WIN32 OR SOLID_BROAD_LIBRARY))

IF(SOLID_FOUND)
  MESSAGE(STATUS "Looking for SOLID - found (${SOLID_SOLID_LIBRARY})")
  SET(SOLID_LIBRARY ${SOLID_SOLID_LIBRARY})
  IF(WIN32)
    SET(SOLID_LIBRARY ${SOLID_LIBRARY} ${SOLID_BROAD_LIBRARY})
  ENDIF(WIN32)
ELSE(SOLID_FOUND)
  MESSAGE(FATAL_ERROR "Could not find SOLID")
ENDIF(SOLID_FOUND)

MARK_AS_ADVANCED(SOLID_SOLIDINCLUDE_DIR)

