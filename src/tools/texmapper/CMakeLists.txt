INCLUDE(../../../cmake/macros.cmake)

PROJECT(texmapper)

SET(TEXMAPPER_SOURCES maintexmapper.cpp ssgSaveAC.cpp)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(tgf tgfclient)
ADD_OSSPEC_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()

#disable developer warning
	if (COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)

ADD_EXECUTABLE(sd-texmapper ${TEXMAPPER_SOURCES})

ADD_PLIB_LIBRARY(sd-texmapper sg ssg ul)

ADD_SDLIB_LIBRARY(sd-texmapper tgf tgfclient)


ADD_PNG_LIBRARY(sd-texmapper)
IF(UNIX)
	ADD_X11_LIBRARY(sd-texmapper)
	ADD_XRANDR_LIBRARY(sd-texmapper)
ENDIF(UNIX)

SD_INSTALL_FILES(BIN TARGETS sd-texmapper)

