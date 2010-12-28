INCLUDE(../../../cmake/macros.cmake)

#PROJECT(raceengineclient)

SET(RACEENGINECLIENT_SOURCES racecareer.cpp raceupdate.cpp racenetwork.cpp racecars.cpp
							 racemessage.cpp racegl.cpp raceinit.cpp racemain.cpp
							 raceweather.cpp raceresults.cpp racesimusimu.cpp 
							 racestate.cpp racesituation.cpp 
                             racemanmenu.cpp raceselectmenu.cpp networkingmenu.cpp
                             racenexteventmenu.cpp raceconfigstate.cpp
							 raceengineclient.h raceenginemenus.h
							 racecareer.h raceupdate.h racenetwork.h racecars.h
							 racemessage.h racegl.h raceinit.h 
							 raceweather.h raceresults.h racesimusimu.h 
							 racestate.h racesituation.h raceselectmenu.h networkingmenu.h)

ADD_SD_DEFINITIONS()

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()

ADD_SDLIB_INCLUDEDIR(math portability robottools tgf tgfclient tgfdata
                     confscreens racescreens networking)

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DRACEENGINECLIENT_DLL)
	
	# Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(raceengineclient SHARED ${RACEENGINECLIENT_SOURCES})

ADD_SDLIB_LIBRARY(raceengineclient
                  tgf tgfclient tgfdata robottools
                  confscreens racescreens networking)

IF(WIN32)
	SD_INSTALL_FILES(BIN TARGETS raceengineclient)
ELSE(WIN32)
	SD_INSTALL_FILES(LIB lib TARGETS raceengineclient)
ENDIF(WIN32)

SD_INSTALL_FILES(DATA config FILES raceresults.xsl)

SD_INSTALL_FILES(DATA config USER config FILES raceengine.xml)

SD_INSTALL_FILES(INCLUDE FILES raceinit.h raceselectmenu.h)

