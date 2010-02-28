INCLUDE(../../../cmake/macros.cmake)

#PROJECT(racescreens)

SET(RACESCREENS_SOURCES driverselect.cpp fileselect.cpp loadingscreen.cpp miscscreens.cpp 
                        pitmenu.cpp racemenu.cpp results.cpp trackselect.cpp
			driver.cpp util.cpp driver.h racescreens.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(portability tgf tgfclient math robottools)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )


IF(UNIX)
        #disable developer warning
        if (COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)
	ADD_LIBRARY(racescreens SHARED ${RACESCREENS_SOURCES})
ENDIF(UNIX)
IF(WIN32)
	ADD_LIBRARY(racescreens STATIC ${RACESCREENS_SOURCES})
ENDIF(WIN32)

ADD_TORCSLIB_LIBRARY(racescreens tgf tgfclient robottools)

ADD_PLIB_LIBRARY(racescreens ul)

IF(UNIX)
	TORCS_INSTALL_FILES(LIB lib TARGETS racescreens)
ENDIF(UNIX)

TORCS_INSTALL_FILES(INCLUDE FILES racescreens.h)

