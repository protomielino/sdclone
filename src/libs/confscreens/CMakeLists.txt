INCLUDE(../../../cmake/macros.cmake)

#PROJECT(confscreens)

SET(CONFSCREENS_HEADERS confscreens.h  controlconfig.h  driverconfig.h
	                graphconfig.h  joystickconfig.h  mouseconfig.h
			openglconfig.h simuconfig.h  soundconfig.h
			aiconfig.h)
SET(CONFSCREENS_SOURCES controlconfig.cpp driverconfig.cpp graphconfig.cpp
                        joystickconfig.cpp mouseconfig.cpp openglconfig.cpp
                        simuconfig.cpp soundconfig.cpp aiconfig.cpp
			${CONFSCREENS_HEADERS})

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(tgf tgfclient math portability raceengineclient)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

IF(WIN32)
	include_directories(src/windows/include)
ENDIF(WIN32)


IF(UNIX)
	#disable developer warning
	IF (COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)
	ADD_LIBRARY(confscreens SHARED ${CONFSCREENS_SOURCES})
ENDIF(UNIX)
IF(WIN32)
	ADD_LIBRARY(confscreens STATIC ${CONFSCREENS_SOURCES})
ENDIF(WIN32)

ADD_TORCSLIB_LIBRARY(confscreens tgf tgfclient)


ADD_PLIB_LIBRARY(confscreens ul)


IF(UNIX)
	TORCS_INSTALL_FILES(LIB lib TARGETS confscreens)
ENDIF(UNIX)
TORCS_INSTALL_FILES(INCLUDE FILES ${CONFSCREENS_HEADERS})

