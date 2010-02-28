INCLUDE(../../cmake/macros.cmake)
PROJECT(speed-dreams)

SET(TORCS_SOURCES main.cpp linuxspec.cpp)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(client tgf tgfclient)

#disable developer warning
IF (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)
ADD_EXECUTABLE(speed-dreams ${TORCS_SOURCES})

ADD_TORCSLIB_LIBRARY(speed-dreams client confscreens learning raceengine racescreens tgf tgfclient)

ADD_PLIB_LIBRARY(speed-dreams sg ssg ul)
TARGET_LINK_LIBRARIES(speed-dreams dl)

TORCS_INSTALL_FILES(BIN TARGETS speed-dreams)
TORCS_INSTALL_FILES(INCLUDE FILES osspec.h)

