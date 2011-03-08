INCLUDE(../../../cmake/macros.cmake)

#PROJECT(racescreens)

SET(RACESCREENS_SOURCES carselect.cpp driverselect.cpp fileselect.cpp
                        loadingscreen.cpp miscscreens.cpp 
                        pitmenu.cpp raceparamsmenu.cpp results.cpp trackselect.cpp
                        racescreens.h
                        racegl.cpp racemessage.cpp raceselectmenu.cpp
                        racemanmenu.cpp racenexteventmenu.cpp networkingmenu.cpp
                        raceconfigstate.cpp
                        racescreens.cpp
                        raceenginemenus.h 
                        racegl.h racemessage.h raceselectmenu.h 
                        networkingmenu.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(portability confscreens networking tgf tgfclient tgfdata math robottools)

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} )

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DRACESCREENS_DLL)
	
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
   CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(racescreens SHARED ${RACESCREENS_SOURCES})

ADD_SDLIB_LIBRARY(racescreens
                  confscreens networking tgf tgfclient tgfdata robottools)

ADD_PLIB_LIBRARY(racescreens ul)

IF(WIN32)
    SD_INSTALL_FILES(BIN TARGETS racescreens)
ELSE(WIN32)
    SD_INSTALL_FILES(LIB lib TARGETS racescreens)
ENDIF(WIN32)

SD_INSTALL_FILES(INCLUDE FILES racescreens.h raceselectmenu.h)

