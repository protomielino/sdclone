INCLUDE(../../../cmake/macros.cmake)
PROJECT(menuview)

SET(MENUVIEW_SOURCES main.cpp previewmenu.cpp)

ADD_SDLIB_INCLUDEDIR(robottools tgf tgfclient math portability)
ADD_OSSPEC_INCLUDEDIR()
ADD_SDL_INCLUDEDIR()
ADD_INTERFACE_INCLUDEDIR()

#disable developer warning
if (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

IF(UNIX)
	ADD_EXECUTABLE(${SD_TOOLS_EXECPREFIX}menuview ${MENUVIEW_SOURCES} ../../linux/linuxspec.cpp)
ENDIF(UNIX)

IF (WIN32)	
	ADD_EXECUTABLE(${SD_TOOLS_EXECPREFIX}menuview ${MENUVIEW_SOURCES} ../../windows/windowsspec.cpp)
ENDIF(WIN32)


ADD_SDLIB_LIBRARY(${SD_TOOLS_EXECPREFIX}menuview robottools tgf tgfclient)


IF(OPENGL_FOUND)
	TARGET_LINK_LIBRARIES(${SD_TOOLS_EXECPREFIX}menuview ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

ADD_PNG_LIBRARY(${SD_TOOLS_EXECPREFIX}menuview)
ADD_SDL_LIBRARY(${SD_TOOLS_EXECPREFIX}menuview)

ADD_PLIB_LIBRARY(${SD_TOOLS_EXECPREFIX}menuview sg ssg ul)
ADD_DL_LIBRARY(${SD_TOOLS_EXECPREFIX}menuview)

SD_INSTALL_FILES(BIN TARGETS ${SD_TOOLS_EXECPREFIX}menuview)

