INCLUDE(../../../cmake/macros.cmake)

#PROJECT(raceengine)

SET(RACEENGINE_SOURCES raceengine.cpp
                       racecareer.cpp raceupdate.cpp racenetwork.cpp racecars.cpp
                       raceinit.cpp racemain.cpp
                       racetrack.cpp raceresults.cpp racesimusimu.cpp 
                       racestate.cpp racesituation.cpp 
                       raceutil.cpp
                       raceengine.h 
                       racecareer.h raceupdate.h racenetwork.h racecars.h
                       raceinit.h 
                       racetrack.h raceresults.h racesimusimu.h 
                       racestate.h racesituation.h 
                       raceutil.h)

ADD_SD_DEFINITIONS()

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()

ADD_SDLIB_INCLUDEDIR(math portability robottools tgf tgfclient tgfdata
                     racescreens networking)

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DRACEENGINE_DLL)
    
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(raceengineclient SHARED ${RACEENGINE_SOURCES})

ADD_SDLIB_LIBRARY(raceengineclient
                  tgf tgfclient tgfdata robottools
                  racescreens networking)

IF(WIN32)
    SD_INSTALL_FILES(BIN TARGETS raceengineclient)
ELSE(WIN32)
    SD_INSTALL_FILES(LIB lib TARGETS raceengineclient)
ENDIF(WIN32)

SD_INSTALL_FILES(DATA config FILES raceresults.xsl)

SD_INSTALL_FILES(DATA config USER config FILES raceengine.xml)

SD_INSTALL_FILES(INCLUDE FILES raceinit.h)

