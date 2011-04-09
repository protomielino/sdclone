INCLUDE(../../../cmake/macros.cmake)

SET(TRACK_SOURCES track.cpp track3.cpp track4.cpp trackitf.cpp trackutil.cpp
                  trackitf.h trackinc.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math tgf robottools)

IF(WIN32)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

ADD_LIBRARY(track SHARED ${TRACK_SOURCES})

IF(UNIX)
    SET_TARGET_PROPERTIES(track PROPERTIES PREFIX "")
ENDIF(UNIX)

ADD_PLIB_LIBRARY(track ul sg)
ADD_SDLIB_LIBRARY(track tgf robottools)

SD_INSTALL_FILES(LIB modules/track TARGETS track)
