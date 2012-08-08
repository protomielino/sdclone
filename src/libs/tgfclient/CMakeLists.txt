INCLUDE(../../../cmake/macros.cmake)

#PROJECT(tgfclient)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(tgf math portability)
ADD_SDL_INCLUDEDIR()
ADD_JPEG_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()


SET(TGFCLIENT_SOURCES control.cpp glfeatures.cpp guibutton.cpp guifont.cpp
                      guiimage.cpp guimenu.cpp guiscrollbar.cpp guitexture.cpp
                      tgfclient.cpp gui.cpp guiedit.cpp guihelp.cpp
                      guilabel.cpp guiobject.cpp guiscrollist.cpp 
                      guicombobox.cpp guicheckbox.cpp guiprogresbar.cpp 
                      guiscreen.cpp guieventloop.cpp guiapplication.cpp 
                      glfeatures.h gui.h guiscreen.h guimenu.h tgfclient.h guifont.h )

#disable developer warning
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DTGFCLIENT_DLL)
ENDIF(WIN32)
    
IF(MSVC)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG 
        "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcmt.lib")
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE 
        "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmt.lib")
ENDIF(MSVC)

ADD_LIBRARY(tgfclient SHARED ${TGFCLIENT_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(tgfclient PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

ADD_PLIB_LIBRARY(tgfclient ul sg sl)
ADD_SDLIB_LIBRARY(tgfclient tgf networking)

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
    SD_INSTALL_FILES(LIB lib TARGETS tgfclient)
ELSE(UNIX)
    SD_INSTALL_FILES(BIN TARGETS tgfclient)
ENDIF(UNIX)

SD_INSTALL_FILES(DATA config USER config FILES screen.xml)

SD_INSTALL_FILES(INCLUDE FILES glfeatures.h guiscreen.h tgfclient.h)

