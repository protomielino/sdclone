INCLUDE(../../../cmake/macros.cmake)
PROJECT(trackgen)

SET(TRACKGEN_SOURCES ac3d.cpp easymesh.cpp elevation.cpp maintrackgen.cpp
	             objects.cpp relief.cpp track.cpp util.cpp
	             ac3d.h easymesh.h elevation.h objects.h relief.h trackgen.h util.h
		     )

ADD_SDLIB_INCLUDEDIR(robottools tgf tgfclient math portability)
ADD_OSSPEC_INCLUDEDIR()
ADD_SDL_INCLUDEDIR()
ADD_INTERFACE_INCLUDEDIR()

#disable developer warning
if (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

IF(WIN32)
	# Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

IF(UNIX)
	ADD_EXECUTABLE(sd-trackgen ${TRACKGEN_SOURCES} ../../linux/linuxspec.cpp)
ENDIF(UNIX)

IF (WIN32)	
	ADD_EXECUTABLE(sd-trackgen ${TRACKGEN_SOURCES} ../../windows/windowsspec.cpp)
ENDIF(WIN32)


ADD_SDLIB_LIBRARY(sd-trackgen robottools tgf tgfclient client)


IF(OPENGL_FOUND)
	TARGET_LINK_LIBRARIES(sd-trackgen ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

ADD_PNG_LIBRARY(sd-trackgen)
ADD_SDL_LIBRARY(sd-trackgen)

ADD_PLIB_LIBRARY(sd-trackgen sg ssg ul)
ADD_DL_LIBRARY(sd-trackgen)

SD_INSTALL_FILES(BIN TARGETS sd-trackgen)

