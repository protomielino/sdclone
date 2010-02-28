INCLUDE(../../../cmake/macros.cmake)

#PROJECT(tgfclient)

ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()

ADD_TORCSLIB_INCLUDEDIR(tgf math portability client networking)
ADD_SDL_INCLUDEDIR()
ADD_JPEG_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()


SET(TGFCLIENT_SOURCES control.cpp glfeatures.cpp guibutton.cpp guifont.cpp
                      guiimage.cpp guimenu.cpp guiscrollbar.cpp guitexture.cpp
                      tgfclient.cpp gui.cpp guiedit.cpp guihelp.cpp
		      guilabel.cpp guiobject.cpp guiscrollist.cpp guicombobox.cpp guicheckbox.cpp guiprogresbar.cpp
		      screen.cpp sdlcallbacks.cpp glfeatures.h gui.h screen_properties.h 
		      guimenu.h sdlcallbacks.h tgfclient.h guifont.h )

#disable developer warning
IF (COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

IF(UNIX)
	ADD_LIBRARY(tgfclient SHARED ${TGFCLIENT_SOURCES})
ENDIF(UNIX)

IF(WIN32)
	ADD_LIBRARY(tgfclient SHARED ${TGFCLIENT_SOURCES} tgfclient.def)
ENDIF(WIN32)

ADD_PLIB_LIBRARY(tgfclient ssg ul sg sl ssg ssgaux)
ADD_TORCSLIB_LIBRARY(tgfclient tgf networking)

ADD_PNG_LIBRARY(tgfclient)
ADD_JPEG_LIBRARY(tgfclient)
IF(OPENGL_FOUND)
	TARGET_LINK_LIBRARIES(tgfclient ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

ADD_PLIB_LIBRARY(tgfclient ul sg js)
ADD_SDL_LIBRARY(tgfclient)

IF(WIN32)
#joystick library
	TARGET_LINK_LIBRARIES(tgfclient winmm)
ENDIF(WIN32)

IF(UNIX)
	ADD_X11_LIBRARY(tgfclient)
	IF(OPTION_XRANDR)
		ADD_XRANDR_LIBRARY(tgfclient)
	ENDIF(OPTION_XRANDR)
ENDIF(UNIX)

IF(UNIX)
	TORCS_INSTALL_FILES(LIB lib TARGETS tgfclient)
ELSE(UNIX)
	TORCS_INSTALL_FILES(BIN TARGETS tgfclient)
ENDIF(UNIX)


TORCS_INSTALL_FILES(DATA config USER config FILES screen.xml)

TORCS_INSTALL_FILES(INCLUDE FILES glfeatures.h screen_properties.h tgfclient.h)

