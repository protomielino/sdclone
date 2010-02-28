#This file can be distributed with packages such as cars, robots, tracks
#It tries to locate a installed torcs-ng version and include the distributed macro's

SET(TORCSNG_PREFIX CACHE PATH "Prefix where Speed Dreams is installed")
SET(TORCSNG_DATADIR CACHE PATH "Place where the data is installed")
FIND_FILE(TORCSNG_CMAKE_MACROS speed-dreams.cmake PATHS ${TORCSNG_PREFIX} ${TORCSNG_PREFIX}/${TORCSNG_DATADIR} ${TORCSNG_DATADIR} /usr /usr/local PATH_SUFFIXES cmake share/cmake share/games/torcs/cmake DOC "Place where Speed Dreams is installed")
IF(NOT TORCSNG_CMAKE_MACROS)
	MESSAGE(FATAL_ERROR "Didn't find Speed Dreams. Please specify the location with the TORCSNG_PREFIX or TORCSNG_DATADIR path.")
ENDIF(NOT TORCSNG_CMAKE_MACROS)

INCLUDE(${TORCSNG_CMAKE_MACROS})

