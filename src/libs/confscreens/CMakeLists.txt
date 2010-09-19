INCLUDE(../../../cmake/macros.cmake)

#PROJECT(confscreens)

SET(CONFSCREENS_HEADERS confscreens.h controlconfig.h playerconfig.h
	                    graphconfig.h joystickconfig.h mouseconfig.h
	                    openglconfig.h simuconfig.h soundconfig.h aiconfig.h
	                    carsettingsmenu.h hostsettingsmenu.h)

SET(CONFSCREENS_SOURCES controlconfig.cpp playerconfig.cpp graphconfig.cpp
                        joystickconfig.cpp mouseconfig.cpp openglconfig.cpp
                        simuconfig.cpp soundconfig.cpp aiconfig.cpp
	                    carsettingsmenu.cpp hostsettingsmenu.cpp
	                    ${CONFSCREENS_HEADERS})

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(tgf tgfclient networking math portability)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

IF(WIN32)
	INCLUDE_DIRECTORIES(src/windows/include)
ENDIF(WIN32)

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DCONFSCREENS_DLL)
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(confscreens SHARED ${CONFSCREENS_SOURCES})

ADD_SDLIB_LIBRARY(confscreens tgf tgfclient networking)

ADD_PLIB_LIBRARY(confscreens ul)

IF(WIN32)
	SD_INSTALL_FILES(BIN TARGETS confscreens)
ELSE(WIN32)
	SD_INSTALL_FILES(LIB lib TARGETS confscreens)
ENDIF(WIN32)

SD_INSTALL_FILES(INCLUDE FILES ${CONFSCREENS_HEADERS})

