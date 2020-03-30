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

#==============================================================================
IF((APPLE) AND ("${CMAKE_INSTALL_PREFIX}" MATCHES "\\.app$"))
 # TODO get these elsewhwere...
   SET(OSG_PLUGIN_VERSION 3.6.4)
   SET(OSG_PLUGIN_DIR /opt/local/lib/osgPlugins-${OSG_PLUGIN_VERSION})

   IF(OPTION_OSGGRAPH)
      FILE(GLOB osg_plugins "${OSG_PLUGIN_DIR}/*.so")
      message("osg_plugins = ${osg_plugins}")
      FOREACH(pi ${osg_plugins})
         INSTALL(FILES ${pi} DESTINATION ./games/PlugIns/osgPlugins-${OSG_PLUGIN_VERSION})
         GET_FILENAME_COMPONENT(fname ${pi} NAME)
         SET_PROPERTY(GLOBAL APPEND PROPERTY SD_OSG_PLUGIN_LIST "${CMAKE_INSTALL_PREFIX}/games/PlugIns/osgPlugins-${OSG_PLUGIN_VERSION}/${fname}")
      ENDFOREACH()
   ENDIF(OPTION_OSGGRAPH)

ENDIF()
#==============================================================================

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
