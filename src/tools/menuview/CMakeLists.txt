INCLUDE(../../../cmake/macros.cmake)
PROJECT(sd-menuview)

SET(MENUVIEW_SOURCES main.cpp previewmenu.cpp)

ADD_TORCSLIB_INCLUDEDIR(client robottools tgf tgfclient math portability)
ADD_OSSPEC_INCLUDEDIR()
ADD_SDL_INCLUDEDIR()
ADD_INTERFACE_INCLUDEDIR()

#disable developer warning
if (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

IF(UNIX)
	ADD_EXECUTABLE(sd-menuview ${MENUVIEW_SOURCES} ../../linux/linuxspec.cpp)
ENDIF(UNIX)

IF (WIN32)	
	ADD_EXECUTABLE(sd-menuview ${MENUVIEW_SOURCES} ../../windows/windowsspec.cpp)
ENDIF(WIN32)


ADD_TORCSLIB_LIBRARY(sd-menuview robottools tgf tgfclient client)


IF(OPENGL_FOUND)
	TARGET_LINK_LIBRARIES(sd-menuview ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

ADD_PNG_LIBRARY(sd-menuview)
ADD_SDL_LIBRARY(sd-menuview)

ADD_PLIB_LIBRARY(sd-menuview sg ssg ul)

TORCS_INSTALL_FILES(BIN TARGETS sd-menuview)

