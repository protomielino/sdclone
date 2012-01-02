INCLUDE(../../../../cmake/macros.cmake)

SET(_SOURCES textonly.cpp textonly.h)

#INCLUDE_DIRECTORIES(.)
ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math portability tgf tgfclient tgfdata networking)
ADD_PLIB_INCLUDEDIR()
ADD_SDL_INCLUDEDIR()
ADD_ENET_INCLUDEDIR()

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DTEXTONLY_DLL)
    
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Disable developer warning
IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(textonly SHARED ${_SOURCES})

SET_TARGET_PROPERTIES(textonly PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(UNIX)
    SET_TARGET_PROPERTIES(textonly PROPERTIES PREFIX "")
ENDIF(UNIX)

ADD_SDLIB_LIBRARY(textonly tgf tgfclient tgfdata networking)

SD_INSTALL_FILES(LIB modules/userinterface TARGETS textonly)
