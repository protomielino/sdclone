INCLUDE(../../../cmake/macros.cmake)

#PROJECT(raceengineclient)

SET(RACEENGINECLIENT_SOURCES racecareer.cpp raceupdate.cpp racenetwork.cpp racecars.cpp
							 racemessage.cpp racegl.cpp raceinit.cpp racemain.cpp
							 raceweather.cpp racemanmenu.cpp raceresults.cpp racesimusimu.cpp 
							 racestate.cpp racesituation.cpp singleplayer.cpp networkingmenu.cpp
							 raceengineclient.h 
							 racecareer.h raceupdate.h racenetwork.h racecars.h
							 racemessage.h racegl.h raceinit.h 
							 raceweather.h racemanmenu.h raceresults.h racesimusimu.h 
							 racestate.h racesituation.h singleplayer.h networkingmenu.h)

ADD_TORCS_DEFINITIONS()

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()

ADD_TORCSLIB_INCLUDEDIR(confscreens math portability racescreens robottools 
						tgf tgfclient networking)

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DRACEENGINECLIENT_DLL)
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(raceengineclient SHARED ${RACEENGINECLIENT_SOURCES})

ADD_TORCSLIB_LIBRARY(raceengineclient confscreens racescreens robottools tgf tgfclient networking)

IF(WIN32)
	TORCS_INSTALL_FILES(BIN TARGETS raceengineclient)
ELSE(WIN32)
	TORCS_INSTALL_FILES(LIB lib TARGETS raceengineclient)
ENDIF(WIN32)

TORCS_INSTALL_FILES(DATA config USER config FILES raceengine.xml)

TORCS_INSTALL_FILES(INCLUDE FILES raceinit.h singleplayer.h)

