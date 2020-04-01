#==============================================================================
#
#   file        : CMakeLists.txt
#   copyright   : (C) 2019 Joe Thompson
#   email       : beaglejoe@users.sourceforge.net
#   web         : www.speed-dreams.org
#   version     : $Id$
#
#==============================================================================
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#==============================================================================
INCLUDE(../cmake/macros.cmake)

IF(OPTION_OSGGRAPH)
   #MESSAGE(STATUS OPENSCENEGRAPH_LIBRARIES = "${OPENSCENEGRAPH_LIBRARIES}")

   LIST(GET OPENSCENEGRAPH_LIBRARIES 0 _OSGLIB)
   #MESSAGE(STATUS _OSGLIB = "${_OSGLIB}")

   GET_FILENAME_COMPONENT(_OSG_LIB_PATH "${_OSGLIB}" PATH)
   #MESSAGE(STATUS _OSG_LIB_PATH = "${_OSG_LIB_PATH}")


   SET(OSG_PLUGIN_DIR osgPlugins-${OPENSCENEGRAPH_VERSION})
   #MESSAGE(STATUS OSG_PLUGIN_DIR = "${OSG_PLUGIN_DIR}")

   if(WIN32)
      SET(_OSG_LIB_PATH "${_OSG_LIB_PATH}/../bin")
      find_file(_OSG_FILE "osgversion${CMAKE_EXECUTABLE_SUFFIX}" PATHS ${_OSG_LIB_PATH})
      GET_FILENAME_COMPONENT(_OSG_LIB_PATH "${_OSG_FILE}" PATH)
      #MESSAGE(STATUS _OSG_FILE = "${_OSG_FILE}")
      unset(_OSG_FILE CACHE )
      #MESSAGE(STATUS _OSG_FILE = "${_OSG_FILE}")
   ENDIF(WIN32)

   #MESSAGE(STATUS _OSG_LIB_PATH = "${_OSG_LIB_PATH}")
   #MESSAGE(STATUS CMAKE_SHARED_LIBRARY_SUFFIX = "${CMAKE_SHARED_LIBRARY_SUFFIX}")
   #MESSAGE(STATUS CMAKE_SHARED_MODULE_SUFFIX = "${CMAKE_SHARED_MODULE_SUFFIX}")
   #MESSAGE(STATUS CMAKE_EXECUTABLE_SUFFIX = "${CMAKE_EXECUTABLE_SUFFIX}")

   MESSAGE(STATUS "Globbing from: ${_OSG_LIB_PATH}/${OSG_PLUGIN_DIR}/*${CMAKE_SHARED_MODULE_SUFFIX}")
   FILE(GLOB OSG_PLUGINS "${_OSG_LIB_PATH}/${OSG_PLUGIN_DIR}/*${CMAKE_SHARED_MODULE_SUFFIX}")
   message("OSG_PLUGINS = ${OSG_PLUGINS}")
ENDIF(OPTION_OSGGRAPH)

#==============================================================================
IF((APPLE) AND ("${CMAKE_INSTALL_PREFIX}" MATCHES "\\.app$"))

   IF(OPTION_OSGGRAPH)
      set(osg_plugins ${OSG_PLUGINS})
      #FILE(GLOB osg_plugins "${_OSG_LIB_PATH}/${OSG_PLUGIN_DIR}/*${CMAKE_SHARED_MODULE_SUFFIX}")
      #message("osg_plugins = ${osg_plugins}")
      FOREACH(pi ${osg_plugins})
         INSTALL(FILES ${pi} DESTINATION ./games/PlugIns/${OSG_PLUGIN_DIR})
         GET_FILENAME_COMPONENT(fname ${pi} NAME)
         SET_PROPERTY(GLOBAL APPEND PROPERTY SD_OSG_PLUGIN_LIST "${CMAKE_INSTALL_PREFIX}/games/PlugIns/${OSG_PLUGIN_DIR}/${fname}")
      ENDFOREACH()
   ENDIF(OPTION_OSGGRAPH)

ENDIF()
#==============================================================================

Message(STATUS "Todo - Check handling of SD_OSG_PLUGIN_LIST_ITEMS in fixbundle.cmake.in")
IF((APPLE) AND ("${CMAKE_INSTALL_PREFIX}" MATCHES "\\.app$"))
   GET_PROPERTY(SD_MODULE_LIST_ITEMS GLOBAL PROPERTY SD_MODULE_LIST)
   GET_PROPERTY(SD_ROBOT_LIST_ITEMS GLOBAL PROPERTY SD_ROBOT_LIST)
   GET_PROPERTY(SD_OSG_PLUGIN_LIST_ITEMS GLOBAL PROPERTY SD_OSG_PLUGIN_LIST)
   #GET_PROPERTY(SD_EXE_LIST_ITEMS GLOBAL PROPERTY SD_EXE_LIST)

   SET (_bundle "${CMAKE_INSTALL_PREFIX}/${SD_BINDIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")

   SET(_loadables )

   foreach(_MODULE ${SD_MODULE_LIST_ITEMS})
      LIST(APPEND _loadables "${CMAKE_INSTALL_PREFIX}/${_MODULE}")
   endforeach()

   foreach(_ROBOT ${SD_ROBOT_LIST_ITEMS})
      LIST(APPEND _loadables "${CMAKE_INSTALL_PREFIX}/${_MODULE}")
   endforeach()

   #================
   # TODO figure out if this is needed...
   IF(NOT OPTION_OFFICIAL_ONLY)
      LIST(APPEND _loadables "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/libephemeris.dylib")
   ENDIF(NOT OPTION_OFFICIAL_ONLY)
   LIST(APPEND _loadables "${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib/liblearning.dylib")
   #================


   CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/fixbundle.cmake.in
                  ${CMAKE_BINARY_DIR}/fixbundle.cmake @ONLY)

   INSTALL(SCRIPT "${CMAKE_BINARY_DIR}/fixbundle.cmake")

   #ADD_CUSTOM_TARGET(${_UNINST_TGT_NAME} "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake" )
ENDIF()
