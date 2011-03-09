INCLUDE(../../cmake/macros.cmake)
PROJECT(speed-dreams)

SET(SD_SOURCES main.cpp linuxspec.cpp)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(client tgf tgfclient raceengine racescreens)

# Disable developer warning
IF (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

ADD_EXECUTABLE(speed-dreams-2 ${SD_SOURCES})

ADD_SDLIB_LIBRARY(speed-dreams-2 client confscreens learning raceengine racescreens tgf tgfclient)

ADD_PLIB_LIBRARY(speed-dreams-2 sg ul)
ADD_DL_LIBRARY(speed-dreams-2)

SD_INSTALL_FILES(BIN TARGETS speed-dreams-2)
SD_INSTALL_FILES(INCLUDE FILES osspec.h)

