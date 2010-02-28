# - Locate ALUT
# This module defines
# ALUT_LIBRARY
# ALUT_FOUND, if false, do not try to link to OpenAL
# ALUT_INCLUDE_DIR, where to find the headers
#
# $OPENALDIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENALDIR
# used in building OpenAL.
#
# Created by Bryan Donlan, based on the FindOpenAL.cmake module by Eric Wang.
 
FIND_PATH(ALUT_INCLUDE_DIR alut.h
  HINTS ENV OPENALDIR
  PATH_SUFFIXES 
	Headers include/AL include/OpenAL include
  PATHS
    ~/Library/Frameworks/OpenAL.framework
    /Library/Frameworks/OpenAL.framework
    /System/Library/Frameworks/OpenAL.framework # Tiger
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt)
# I'm not sure if I should do a special casing for Apple. It is
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?),
# do they want the -framework option also?
IF(${ALUT_INCLUDE_DIR} MATCHES ".framework")
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" ALUT_FRAMEWORK_PATH_TMP ${ALUT_INCLUDE_DIR})
  IF("${ALUT_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${ALUT_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET (ALUT_LIBRARY "-framework OpenAL" CACHE STRING "OpenAL framework for OSX")
  ELSE("${ALUT_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${ALUT_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(ALUT_LIBRARY "-F${ALUT_FRAMEWORK_PATH_TMP} -framework OpenAL" CACHE STRING "OpenAL framework for OSX")
  ENDIF("${ALUT_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
    OR "${ALUT_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(ALUT_FRAMEWORK_PATH_TMP "" CACHE INTERNAL "")
 
ELSE(${ALUT_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(ALUT_LIBRARY
  NAMES alut
  HINTS ENV OPENALDIR
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt)
ENDIF(${ALUT_INCLUDE_DIR} MATCHES ".framework")
 
SET(ALUT_FOUND "NO")
IF(ALUT_LIBRARY)
  SET(ALUT_FOUND "YES")
  #MESSAGE(STATUS "Found ALUT: ${ALUT_LIBRARY}")
ENDIF(ALUT_LIBRARY)

