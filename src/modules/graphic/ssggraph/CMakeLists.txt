INCLUDE(../../../../cmake/macros.cmake)

SET(SSGGRAPH_SOURCES CarSoundData.cpp CarSoundData.h 
                     Sound.cpp Sound.h PlibSound.cpp PlibSound.h OpenalSound.cpp OpenalSound.h
                     SoundInterface.cpp SoundInterface.h
                     PlibSoundInterface.cpp PlibSoundInterface.h 
                     OpenalSoundInterface.cpp OpenalSoundInterface.h 
                     grCelestialBody.cpp grCloudLayer.cpp grSky.cpp grSky.h
                     grSkyDome.cpp grSphere.cpp grSphere.h grStars.cpp
                     grboard.cpp grboard.h grcam.cpp grcam.h grcar.cpp grcar.h
                     grcarlight.cpp grcarlight.h grmain.cpp grmain.h 
                     grsimplestate.cpp grsimplestate.h 
                     grmultitexstate.cpp grmultitexstate.h 
                     grloadac.cpp grloadac.h grssgext.h
                     grscene.cpp grscene.h grbackground.cpp grbackground.h 
                     grscreen.cpp grscreen.h 
                     grshadow.cpp grshadow.h grskidmarks.cpp grskidmarks.h
                     grsmoke.cpp grsmoke.h grtexture.cpp grtexture.h loadsgi.h
                     grsound.cpp grsound.h grtracklight.cpp grtracklight.h 
                     grtrackmap.cpp grtrackmap.h grutil.cpp grutil.h
                     grvtxtable.cpp grvtxtable.h grrain.cpp grrain.h
                     ssggraph.cpp ssggraph.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(portability math tgf tgfclient robottools)
ADD_SDL_INCLUDEDIR()
ADD_OPENAL_INCLUDEDIR()

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DSSGGRAPH_DLL)

    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

ADD_LIBRARY(ssggraph SHARED ${SSGGRAPH_SOURCES})

SET_TARGET_PROPERTIES(ssggraph PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(UNIX)
    SET_TARGET_PROPERTIES(ssggraph PROPERTIES PREFIX "")
ENDIF(UNIX)

ADD_PLIB_LIBRARY(ssggraph ul sg sl ssg ssgaux)
ADD_SDL_LIBRARY(ssggraph)

ADD_SDLIB_LIBRARY(ssggraph tgf tgfclient robottools)

IF(OPENAL_FOUND)
    TARGET_LINK_LIBRARIES(ssggraph ${OPENAL_LIBRARY})
ENDIF(OPENAL_FOUND)

IF(OPENGL_FOUND)
    TARGET_LINK_LIBRARIES(ssggraph ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

SD_INSTALL_FILES(DATA config USER config FILES graph.xml sound.xml)
SD_INSTALL_FILES(LIB modules/graphic TARGETS ssggraph)
