INCLUDE(../../../../cmake/macros.cmake)

SET(SNDDEFAULT_SOURCES  snddefault.cpp snddefault.h 
                     CarSoundData.cpp CarSoundData.h 
                     Sound.cpp Sound.h PlibSound.cpp PlibSound.h OpenalSound.cpp OpenalSound.h
                     SoundInterface.cpp SoundInterface.h
                     PlibSoundInterface.cpp PlibSoundInterface.h 
                     OpenalSoundInterface.cpp OpenalSoundInterface.h 
                     grsound.cpp grsound.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(portability math tgf tgfclient robottools)
ADD_SDL_INCLUDEDIR()
ADD_OPENAL_INCLUDEDIR()

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DSNDDEFAULT_DLL)

    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

ADD_LIBRARY(snddefault SHARED ${SNDDEFAULT_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(ssggraph PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(UNIX)
    SET_TARGET_PROPERTIES(snddefault PROPERTIES PREFIX "")
ENDIF(UNIX)

ADD_PLIB_LIBRARY(snddefault ul sg sl ssg ssgaux)
ADD_SDL_LIBRARY(snddefault)

ADD_SDLIB_LIBRARY(snddefault tgf tgfclient robottools)

IF(OPENAL_FOUND)
    TARGET_LINK_LIBRARIES(snddefault ${OPENAL_LIBRARY})
ENDIF(OPENAL_FOUND)

IF(OPENGL_FOUND)
    TARGET_LINK_LIBRARIES(snddefault ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

SD_INSTALL_FILES(DATA config USER config FILES sound.xml)
SD_INSTALL_FILES(LIB modules/sound TARGETS snddefault)
