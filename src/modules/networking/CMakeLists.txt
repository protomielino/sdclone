INCLUDE(../../../cmake/macros.cmake)

#PROJECT(networking)

SET(SD_NETWORK_SOURCES network.h network.cpp server.cpp client.cpp robotxml.cpp robotxml.h)

#disable developer warning
if (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

ADD_LIBRARY(networking SHARED ${SD_NETWORK_SOURCES})

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_SDL_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(networking portability tgf math)

ADD_PLIB_INCLUDEDIR()
ADD_ENET_INCLUDEDIR()
ADD_PLIB_LIBRARY(networking ul sg)
ADD_ENET_LIBRARY(networking)
ADD_SDL_LIBRARY(networking)

IF(WIN32)
	TARGET_LINK_LIBRARIES(networking Winmm ws2_32)
ENDIF(WIN32)


ADD_SDLIB_LIBRARY(networking tgf)

IF(WIN32)
	SD_INSTALL_FILES(BIN TARGETS networking)
ELSE(WIN32)
	SD_INSTALL_FILES(LIB lib TARGETS networking)
ENDIF(WIN32)
