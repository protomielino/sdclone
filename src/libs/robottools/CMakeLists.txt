INCLUDE(../../../cmake/macros.cmake)

#PROJECT(robottools)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(portability tgf math)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

SET(ROBOTTOOLS_SOURCES rtteammanager.cpp rttelem.cpp rttimeanalysis.cpp  rttrack.cpp rtutil.cpp
					   robottools.h teammanager.h timeanalysis.h)


IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DROBOTTOOLS_DLL)

	# Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(robottools SHARED ${ROBOTTOOLS_SOURCES})

SET_TARGET_PROPERTIES(robottools PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

ADD_PLIB_LIBRARY(robottools sg ul)
ADD_SDLIB_LIBRARY(robottools tgf)

IF(WIN32)
	SD_INSTALL_FILES(BIN TARGETS robottools)
ELSE(WIN32)
	SD_INSTALL_FILES(LIB lib TARGETS robottools)
ENDIF(WIN32)
SD_INSTALL_FILES(INCLUDE FILES robottools.h)
SD_INSTALL_FILES(INCLUDE FILES teammanager.h)

