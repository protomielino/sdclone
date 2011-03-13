INCLUDE(../../../cmake/macros.cmake)

PROJECT(trackgen)

SET(TRACKGEN_SOURCES ac3d.cpp easymesh.cpp elevation.cpp maintrackgen.cpp
	             objects.cpp relief.cpp track.cpp util.cpp
	             ac3d.h easymesh.h elevation.h objects.h relief.h trackgen.h util.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(robottools tgf tgfclient math portability)
ADD_SDL_INCLUDEDIR()

#disable developer warning
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
	# Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

ADD_EXECUTABLE(${SD_TOOLS_EXECPREFIX}trackgen ${TRACKGEN_SOURCES})

ADD_SDLIB_LIBRARY(${SD_TOOLS_EXECPREFIX}trackgen robottools tgf tgfclient)

IF(OPENGL_FOUND)
    TARGET_LINK_LIBRARIES(${SD_TOOLS_EXECPREFIX}trackgen ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

ADD_PNG_LIBRARY(${SD_TOOLS_EXECPREFIX}trackgen)
ADD_SDL_LIBRARY(${SD_TOOLS_EXECPREFIX}trackgen)
ADD_PLIB_LIBRARY(${SD_TOOLS_EXECPREFIX}trackgen sg ssg ul)
ADD_DL_LIBRARY(${SD_TOOLS_EXECPREFIX}trackgen)

SD_INSTALL_FILES(BIN TARGETS ${SD_TOOLS_EXECPREFIX}trackgen)

