INCLUDE(../../../cmake/macros.cmake)

#PROJECT(client)

SET(CLIENT_SOURCES creditsmenu.cpp entry.cpp exitmenu.cpp mainmenu.cpp
                   optionmenu.cpp splash.cpp 
                   client.h creditsmenu.h exitmenu.h mainmenu.h
		   optionmenu.h splash.h networkingmenu.cpp networkingmenu.h
		   hostsettingsmenu.h hostsettingsmenu.cpp
		   carsettingsmenu.h carsettingsmenu.cpp
		   carinfo.h carinfo.cpp
		   menuscreen.h)
IF(WIN32)
	#errno.cpp should only be included in Windows
	#(it failes to compile when added in Linux)
	#It also adds client.def to the sources for Windows
	SET(CLIENT_SOURCES ${CLIENT_SOURCES} errno.cpp client.def)
ENDIF(WIN32)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(confscreens tgf tgfclient raceengineclient racescreens math networking)
ADD_SDL_INCLUDEDIR()
ADD_ENET_INCLUDEDIR()

#disable developer warning
if (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

ADD_LIBRARY(client SHARED ${CLIENT_SOURCES})

#ADD_PLIB_LIBRARY(client ssg ul sg sl ssg ssgaux)
ADD_TORCSLIB_LIBRARY(client confscreens tgfclient raceengine racescreens track ssggraph robottools networking)
#ADD_PNG_LIBRARY(client)
ADD_OPENGL_LIBRARY(client)
ADD_SDL_LIBRARY(client)
ADD_OPENAL_LIBRARY(client)
ADD_ALUT_LIBRARY(client)

IF(WIN32)
	#joystick library
	TARGET_LINK_LIBRARIES(client winmm)
ENDIF(WIN32)

IF(WIN32)
	TORCS_INSTALL_FILES(BIN TARGETS client)
ELSE(WIN32)
	TORCS_INSTALL_FILES(LIB lib TARGETS client)
ENDIF(WIN32)
TORCS_INSTALL_FILES(INCLUDE FILES client.h exitmenu.h)

