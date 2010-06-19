INCLUDE(../../../cmake/macros.cmake)

#PROJECT(robottools)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(portability tgf math)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

SET(ROBOTTOOLS_SOURCES rtteammanager.cpp rttelem.cpp rttimeanalysis.cpp  rttrack.cpp rtutil.cpp
					   robottools.h teammanager.h timeanalysis.h)


IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DROBOTTOOLS_DLL)
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(robottools SHARED ${ROBOTTOOLS_SOURCES})

ADD_PLIB_LIBRARY(robottools sg ul)
ADD_TORCSLIB_LIBRARY(robottools tgf)

IF(WIN32)
	TORCS_INSTALL_FILES(BIN TARGETS robottools)
ELSE(WIN32)
	TORCS_INSTALL_FILES(LIB lib TARGETS robottools)
ENDIF(WIN32)
TORCS_INSTALL_FILES(INCLUDE FILES robottools.h)
TORCS_INSTALL_FILES(INCLUDE FILES teammanager.h)

