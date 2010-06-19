INCLUDE(../../../cmake/macros.cmake)

#PROJECT(client)

SET(CLIENT_SOURCES creditsmenu.cpp entry.cpp exitmenu.cpp mainmenu.cpp
                   optionmenu.cpp splash.cpp 
                   client.h creditsmenu.h exitmenu.h mainmenu.h
                   optionmenu.h splash.h)
IF(WIN32)
	#errno.cpp should only be included in Windows
	#(it fails to compile when added in Linux)
	SET(CLIENT_SOURCES ${CLIENT_SOURCES} errno.cpp)
ENDIF(WIN32)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(confscreens tgf tgfclient raceengineclient racescreens math)
ADD_SDL_INCLUDEDIR()
ADD_ENET_INCLUDEDIR()

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DCLIENT_DLL)
ENDIF(WIN32)

ADD_LIBRARY(client SHARED ${CLIENT_SOURCES})

#ADD_PLIB_LIBRARY(client ssg ul sg sl ssg ssgaux)
ADD_TORCSLIB_LIBRARY(client confscreens tgfclient raceengineclient racescreens track ssggraph robottools)

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

