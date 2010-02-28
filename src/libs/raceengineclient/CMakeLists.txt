INCLUDE(../../../cmake/macros.cmake)

#PROJECT(raceengineclient)

SET(RACEENGINECLIENT_SOURCES racecareer.cpp raceengine.cpp racegl.cpp raceinit.cpp racemain.cpp
			     raceweather.cpp
	                     racemanmenu.cpp raceresults.cpp racesimusimu.cpp racestate.cpp
			     singleplayer.cpp racecareer.h raceweather.h raceengine.h racegl.h raceinit.h racemanmenu.h raceresults.h
			     racesimusimu.h racestate.h singleplayer.h)


ADD_TORCS_DEFINITIONS()

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()

ADD_TORCSLIB_INCLUDEDIR(client confscreens math portability racescreens robottools tgf tgfclient networking)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )


IF(UNIX)
	#disable developer warning
	if (COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)
	ADD_LIBRARY(raceengine SHARED ${RACEENGINECLIENT_SOURCES})
ENDIF(UNIX)
IF(WIN32 OR APPLE)
	ADD_LIBRARY(raceengine STATIC ${RACEENGINECLIENT_SOURCES})
ENDIF(WIN32 OR APPLE)

ADD_TORCSLIB_LIBRARY(raceengine confscreens racescreens robottools tgf tgfclient networking)

IF(UNIX)
	TORCS_INSTALL_FILES(LIB lib TARGETS raceengine)
ENDIF(UNIX)
TORCS_INSTALL_FILES(DATA config USER config FILES raceengine.xml)
TORCS_INSTALL_FILES(INCLUDE FILES raceinit.h singleplayer.h)

