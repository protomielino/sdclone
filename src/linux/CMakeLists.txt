INCLUDE(../../cmake/macros.cmake)
PROJECT(speed-dreams)

SET(SD_SOURCES main.cpp)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(raceengine tgf tgfclient)

# Disable developer warning
IF (COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

ADD_EXECUTABLE(speed-dreams-2 ${SD_SOURCES})

ADD_SDLIB_LIBRARY(speed-dreams-2 raceengine tgf)

ADD_DL_LIBRARY(speed-dreams-2)

SD_INSTALL_FILES(BIN TARGETS speed-dreams-2)

