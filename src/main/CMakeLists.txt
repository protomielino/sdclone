INCLUDE(../../cmake/macros.cmake)

PROJECT(speed-dreams-2)

SET(_TARGET_NAME speed-dreams-2)

SET(_SOURCES main.cpp)
IF(MSVC)
	SET(_SOURCES ${_SOURCES} main.rc resource.h ../../data/data/icons/icon.ico)
ENDIF(MSVC)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(tgf tgfclient tgfdata math portability)

IF(OPTION_SDL2)
	ADD_SDL2_INCLUDEDIR()
ELSE(OPTION_SDL2)
	ADD_SDL_INCLUDEDIR()
ENDIF(OPTION_SDL2)

ADD_PLIB_INCLUDEDIR()

# Disable developer warning
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(MSVC)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG 
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcmt.lib")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE 
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmt.lib")
ENDIF(MSVC)

SD_ADD_EXECUTABLE(${_TARGET_NAME} ${_SOURCES})

ADD_SDLIB_LIBRARY(${_TARGET_NAME} portability tgf tgfclient tgfdata)

# Needed when using the "MinGW Makefiles" CMake generator (but not with the "MSYS Makefiles" one !)
IF(MINGW)
	TARGET_LINK_LIBRARIES(${_TARGET_NAME} ${MINGW32_LIBRARY})
ENDIF(MINGW)

IF(UNIX)
	SD_INSTALL_FILES(MAN man6 PREFIX ${SOURCE_DIR}/doc/man FILES ${_TARGET_NAME}.6)
ENDIF(UNIX)

SD_INSTALL_FILES(BIN TARGETS ${_TARGET_NAME})

GET_PROPERTY(SD_MODULE_LIST_ITEMS GLOBAL PROPERTY SD_MODULE_LIST)
GET_PROPERTY(SD_ROBOT_LIST_ITEMS GLOBAL PROPERTY SD_ROBOT_LIST)
GET_PROPERTY(SD_OSG_PLUGIN_LIST_ITEMS GLOBAL PROPERTY SD_OSG_PLUGIN_LIST)



IF((APPLE) AND ("${CMAKE_INSTALL_PREFIX}" MATCHES "\\.app$"))
   SET(OSG_PLUGIN_VERSION 3.6.4)
SET(OSG_PLUGIN_DIR /opt/local/lib/osgPlugins-${OSG_PLUGIN_VERSION})
   message("OSG_PLUGIN_DIR = ${OSG_PLUGIN_DIR}")
   INSTALL(FILES ../../packaging/OSX/Info.plist DESTINATION ./Contents)
INSTALL(FILES ../../packaging/OSX/sd-icon-mac-256.icns DESTINATION ./Contents/Resources)
   SET(APPS "\${CMAKE_INSTALL_PREFIX}/games/${_TARGET_NAME}")
   SET(DIRS )
   SET(PlugIns )
#SET(DIRS ${DIRS} "${OSG_PLUGIN_DIR}")
   
   # need to switch on CMake version 3.8.0 ??
   # use MacOS before 3.8.0
   # use Frameworks from 3.8.0 onward
   SET(DIRS ${DIRS} "\${CMAKE_INSTALL_PREFIX}/MacOS")
   SET(DIRS ${DIRS} "\${CMAKE_INSTALL_PREFIX}/Frameworks")


   SET(DIRS ${DIRS} "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib")
   SET(DIRS ${DIRS} /opt/local/lib)

   # TODO need to fixup some of these..
   # These should be pulled into "\${CMAKE_INSTALL_PREFIX}/MacOS" as needed by others
IF(NOT OPTION_OFFICIAL_ONLY)
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/libephemeris.dylib")
ENDIF(NOT OPTION_OFFICIAL_ONLY)

LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/liblearning.dylib")
#LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/libnetworking.dylib")
#LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/libportability.dylib")
#LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/librobottools.dylib")
#LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/libtgf.dylib")
#LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/libtgfclient.dylib")
#LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/libtgfdata.dylib")
ENDIF((APPLE) AND ("${CMAKE_INSTALL_PREFIX}" MATCHES "\\.app$"))

foreach(_MODULE ${SD_MODULE_LIST_ITEMS})
   #message(STATUS "\${CMAKE_INSTALL_PREFIX}/${_MODULE}")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/${_MODULE}")
endforeach()

foreach(_ROBOT ${SD_ROBOT_LIST_ITEMS})
   #message(STATUS "\${CMAKE_INSTALL_PREFIX}/${_ROBOT}")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/${_ROBOT}")
endforeach()

IF((APPLE) AND ("${CMAKE_INSTALL_PREFIX}" MATCHES "\\.app$"))

   IF(OPTION_OSGGRAPH)
#SET(OSG_PLUGIN_DIR /opt/local/lib/osgPlugins-${OSG_PLUGIN_VERSION})
#INSTALL(DIRECTORY "${OSG_PLUGIN_DIR}" DESTINATION ./MacOS FILES_MATCHING PATTERN "*.so")
      LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/graphic/osggraph.so")
      FILE(GLOB osg_plugins "${OSG_PLUGIN_DIR}/*.so")
message("osg_plugins = ${osg_plugins}")
      FOREACH(pi ${osg_plugins})
INSTALL(FILES ${pi} DESTINATION ./games/PlugIns/osgPlugins-${OSG_PLUGIN_VERSION})
         GET_FILENAME_COMPONENT(fname ${pi} NAME)
        message("fname = ${fname}")
         LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/games/PlugIns/osgPlugins-${OSG_PLUGIN_VERSION}/${fname}")
      ENDFOREACH()
   ENDIF(OPTION_OSGGRAPH)

   message("PlugIns = ${PlugIns}")
message("DIRS = ${DIRS}")


# These are installed by their respective CMakeLists.txt
# Executables
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-accc)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-menuview)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-nfs2ac)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-nfsperf)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-sd2-trackgen)

   INSTALL(CODE "
   INCLUDE(BundleUtilities)
   SET(BU_CHMOD_BUNDLE_ITEMS ON)
   FIXUP_BUNDLE(\"${APPS}\" \"${PlugIns}\" \"${DIRS}\")")

ENDIF()

# Under Windows, install needed 3rd party DLLs close to Speed Dreams executable.
IF(WIN32)
	INCLUDE(customthirdparty)
	SD_INSTALL_CUSTOM_3RDPARTY(${_TARGET_NAME})
ENDIF(WIN32)
