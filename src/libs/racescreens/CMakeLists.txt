INCLUDE(../../../cmake/macros.cmake)

#PROJECT(racescreens)

SET(RACESCREENS_SOURCES carselect.cpp driverselect.cpp fileselect.cpp
                        loadingscreen.cpp miscscreens.cpp 
                        pitmenu.cpp racemenu.cpp results.cpp trackselect.cpp
                        driver.cpp util.cpp driver.h racescreens.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(portability tgf tgfclient math robottools)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DRACESCREENS_DLL)
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
   CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(racescreens SHARED ${RACESCREENS_SOURCES})

ADD_TORCSLIB_LIBRARY(racescreens tgf tgfclient robottools)

ADD_PLIB_LIBRARY(racescreens ul)

IF(WIN32)
	TORCS_INSTALL_FILES(BIN TARGETS racescreens)
ELSE(WIN32)
	TORCS_INSTALL_FILES(LIB lib TARGETS racescreens)
ENDIF(WIN32)

TORCS_INSTALL_FILES(INCLUDE FILES racescreens.h)

