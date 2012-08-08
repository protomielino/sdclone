INCLUDE(../../cmake/macros.cmake)

PROJECT(speed-dreams-2)

SET(_TARGET_NAME speed-dreams-2)

SET(_SOURCES main.cpp)
IF(MSVC)
    SET(_SOURCES ${_SOURCES} main.rc resource.h ../../data/data/icons/icon.ico)
ENDIF(MSVC)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(tgf tgfclient tgfdata raceengine math portability)
ADD_SDL_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()

# Disable developer warning
IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_EXECUTABLE(${_TARGET_NAME} ${_SOURCES})

ADD_SDLIB_LIBRARY(${_TARGET_NAME} tgf tgfclient tgfdata raceengine)

IF(UNIX)
    SD_INSTALL_FILES(MAN man6 PREFIX ${SOURCE_DIR}/doc/man FILES ${_TARGET_NAME}.6)
ENDIF(UNIX)

SD_INSTALL_FILES(BIN TARGETS ${_TARGET_NAME})

# Under Windows, install needed 3rd party DLLs close to Speed Dreams executable.
IF(WIN32)
	INCLUDE(Find3rdPartyDependencies)
    SD_INSTALL_3RDPARTY()
ENDIF(WIN32)
