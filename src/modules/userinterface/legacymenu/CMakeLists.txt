INCLUDE(../../../../cmake/macros.cmake)

SET(MS mainscreens)
SET(MAINSCREENS_SOURCES ${MS}/splash.cpp ${MS}/mainmenu.cpp
                        ${MS}/optionsmenu.cpp ${MS}/creditsmenu.cpp ${MS}/exitmenu.cpp 
                        ${MS}/splash.h ${MS}/mainmenu.h
                        ${MS}/optionsmenu.h ${MS}/creditsmenu.h ${MS}/exitmenu.h)

IF(WIN32)
	#errno.cpp should only be included under Windows
	#(it fails to compile when added under Linux)
	SET(MAINSCREENS_SOURCES ${MAINSCREENS_SOURCES} ${MS}/errno.cpp)
ENDIF(WIN32)

SET(CS confscreens)
SET(CONFSCREENS_SOURCES ${CS}/playerconfig.cpp ${CS}/controlconfig.cpp
                        ${CS}/joystickconfig.cpp ${CS}/mouseconfig.cpp
			${CS}/joy2butconfig.cpp
                        ${CS}/displayconfig.cpp ${CS}/graphconfig.cpp ${CS}/openglconfig.cpp
			${CS}/monitorconfig.cpp ${CS}/advancedgraphconfig.cpp
                        ${CS}/simuconfig.cpp ${CS}/soundconfig.cpp ${CS}/aiconfig.cpp
                        ${CS}/carsettingsmenu.cpp ${CS}/hostsettingsmenu.cpp
                        ${CS}/confscreens.h ${CS}/advancedgraphconfig.h
                        ${CS}/playerconfig.h ${CS}/controlconfig.h
			${CS}/joystickconfig.h ${CS}/mouseconfig.h
			${CS}/joy2butconfig.h ${CS}/
                        ${CS}/displayconfig.h ${CS}/graphconfig.h ${CS}/openglconfig.h 
                        ${CS}/simuconfig.h ${CS}/soundconfig.h ${CS}/aiconfig.h
                        ${CS}/carsettingsmenu.h ${CS}/hostsettingsmenu.h)

SET(RS racescreens)
SET(RACESCREENS_SOURCES ${RS}/raceselectmenu.cpp
			#${RS}/raceweekendmenu.cpp
                        ${RS}/racemanmenu.cpp ${RS}/fileselect.cpp ${RS}/networkingmenu.cpp
                        ${RS}/raceconfigstate.cpp
                        ${RS}/trackselect.cpp ${RS}/driverselect.cpp ${RS}/garagemenu.cpp
                        ${RS}/raceparamsmenu.cpp
			${RS}/raceloadingmenu.cpp ${RS}/racerunningmenus.cpp
                        ${RS}/racestopmenu.cpp ${RS}/racestartmenu.cpp ${RS}/racepitmenu.cpp 
                        ${RS}/raceresultsmenus.cpp ${RS}/racenexteventmenu.cpp
                        ${RS}/racescreens.h ${RS}/garagemenu.h) #${RS}/raceweekendscreens.h)

SET(LEGACYMENU_SOURCES ${MAINSCREENS_SOURCES} 
                       ${CONFSCREENS_SOURCES} 
                       ${RACESCREENS_SOURCES}
                       legacymenu.cpp legacymenu.h)

INCLUDE_DIRECTORIES(. ${MS} ${CS} ${RS})
ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math portability tgf tgfclient tgfdata networking)
ADD_PLIB_INCLUDEDIR()
ADD_SDL_INCLUDEDIR()
ADD_ENET_INCLUDEDIR()

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DLEGACYMENU_DLL)
    
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Disable developer warning
IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(legacymenu SHARED ${LEGACYMENU_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(legacymenu PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(UNIX)
    SET_TARGET_PROPERTIES(legacymenu PROPERTIES PREFIX "")
ENDIF(UNIX)

IF(WIN32)
    # Joystick library
    TARGET_LINK_LIBRARIES(legacymenu winmm)
ENDIF(WIN32)

ADD_SDLIB_LIBRARY(legacymenu tgf tgfclient tgfdata networking)

SD_INSTALL_FILES(LIB modules/userinterface TARGETS legacymenu)
