INCLUDE(../../../cmake/macros.cmake)

#PROJECT(metar)

SET(METAR_HEADERS mchannel.h msocket.h mexception.h mmetar.h )
SET(METAR_SOURCES mchannel.cpp msocket.cpp mexception.cpp mmetar.cpp 
			${LEARNING_HEADERS})

ADD_SDLIB_INCLUDEDIR(portability math)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DMETAR_DLL)
ENDIF(WIN32)

ADD_LIBRARY(metar SHARED ${METAR_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(learning PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(WIN32)
	SD_INSTALL_FILES(BIN TARGETS metar)
ELSE(WIN32)
	SD_INSTALL_FILES(LIB lib TARGETS metar)
ENDIF(WIN32)

SD_INSTALL_FILES(INCLUDE metar FILES ${METAR_HEADERS})
