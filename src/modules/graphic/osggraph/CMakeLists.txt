INCLUDE(../../../../cmake/macros.cmake)

SET(SSGGRAPH_HEADERS grmain.h osggraph.h)
SET(SSGGRAPH_SOURCES grmain.cpp osggraph.cpp)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(portability math tgf tgfclient robottools)
ADD_SDL_INCLUDEDIR()

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DOSGGRAPH_DLL)

    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(ssggraph PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(UNIX OR MINGW)
    SET_TARGET_PROPERTIES(osggraph PROPERTIES PREFIX "")
ENDIF(UNIX OR MINGW)

ADD_LIBRARY(osggraph SHARED ${OSGGRAPH_SOURCES})

ADD_OSG_LIBRARY(osggraph osgviewer osgFX osgGA osgUtils osgDB OpenThreads osg)
ADD_SDL_LIBRARY(osggraph)

ADD_SDLIB_LIBRARY(osggraph tgf tgfclient robottools)

find_package(OpenSceneGraph REQUIRED osgDB osgViewer osgGA osgUtil osgFX)
# libOpenThreads & libosg automatically searched
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

target_link_libraries(osggraph ${OPENSCENEGRAPH_LIBRARIES})

IF(OPENAL_FOUND)
	TARGET_LINK_LIBRARIES(osggraph ${OPENAL_LIBRARY})
ENDIF(OPENAL_FOUND)

IF(OPENGL_FOUND)
	TARGET_LINK_LIBRARIES(osggraph ${OPENGL_LIBRARY})
ENDIF(OPENGL_FOUND)

SD_INSTALL_FILES(LIB modules/graphic TARGETS osggraph)