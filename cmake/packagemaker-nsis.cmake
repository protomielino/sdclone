#==============================================================================
#
#   file        : .cmake
#   copyright   : (C) 2019 Joe Thompson
#   email       : beaglejoe@users.sourceforge.net
#   web         : www.speed-dreams.org
#   version     : $Id:$
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
FIND_PACKAGE(NSIS)

Message(STATUS "NSIS_FOUND = ${NSIS_FOUND}")
Message(STATUS "NSIS_MAKE_EXE = ${NSIS_MAKE_EXE}")
Message(STATUS "NSIS_INET_PLUGIN = ${NSIS_INET_PLUGIN}")


if(NSIS_FOUND AND NSIS_MAKE_EXE AND NSIS_INET_PLUGIN)
   FILE(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}" NSIS_INSTALL_DIR)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/readme_for_user.txt" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/readme_for_user.txt" COPYONLY)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/speed-dreams.ini" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/speed-dreams.ini" COPYONLY)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/speed-dreams.nsh.in" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/speed-dreams.nsh" @ONLY)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/speed-dreams-base.nsi" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/speed-dreams-base.nsi" @ONLY)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/speed-dreams-hq-cars-and-tracks.nsi" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/speed-dreams-hq-cars-and-tracks.nsi" @ONLY)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/speed-dreams-more-hq-cars-and-tracks.nsi" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/speed-dreams-more-hq-cars-and-tracks.nsi" @ONLY)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/speed-dreams-wip-cars-and-tracks.nsi" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/speed-dreams-wip-cars-and-tracks.nsi" @ONLY)

   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/speed-dreams-unmaintained.nsi" 
                  "${CMAKE_CURRENT_BINARY_DIR}/packaging/speed-dreams-unmaintained.nsi" @ONLY)

   #ADD_CUSTOM_TARGET(PACKAGE_BASE DEPENDS INSTALL)
      # can't depend on built-in target INSTALL, so we ADD_CUSTOM_COMMAND
      # below to make sure INSTALL is done
   ADD_CUSTOM_TARGET(PACKING_INSTALL)

   ADD_CUSTOM_TARGET(PACKAGE_ALL DEPENDS PACKAGE_BASE
                                         PACKAGE_HQ
                                         PACKAGE_MORE_HQ
                                         PACKAGE_WIP
                                         PACKAGE_UNMAINTAINED)

   ADD_CUSTOM_TARGET(PACKAGE_BASE DEPENDS PACKING_INSTALL)
   ADD_CUSTOM_TARGET(PACKAGE_HQ DEPENDS PACKING_INSTALL)
   ADD_CUSTOM_TARGET(PACKAGE_MORE_HQ DEPENDS PACKING_INSTALL)
   ADD_CUSTOM_TARGET(PACKAGE_WIP DEPENDS PACKING_INSTALL)
   ADD_CUSTOM_TARGET(PACKAGE_UNMAINTAINED DEPENDS PACKING_INSTALL)
   
   ADD_CUSTOM_COMMAND(TARGET PACKING_INSTALL
                      COMMAND "${CMAKE_COMMAND}" --build . --target INSTALL --config $<CONFIG>
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Building INSTALL...")

   ADD_CUSTOM_COMMAND(TARGET PACKAGE_BASE
                        COMMAND ${NSIS_MAKE_EXE} speed-dreams-base.nsi
                        WORKING_DIRECTORY packaging
                        COMMENT "Building base package...")

   ADD_CUSTOM_COMMAND(TARGET PACKAGE_HQ
                        COMMAND ${NSIS_MAKE_EXE} speed-dreams-hq-cars-and-tracks.nsi
                        WORKING_DIRECTORY packaging
                        COMMENT "Building HQ package...")

   ADD_CUSTOM_COMMAND(TARGET PACKAGE_MORE_HQ
                        COMMAND ${NSIS_MAKE_EXE} speed-dreams-more-hq-cars-and-tracks.nsi
                        WORKING_DIRECTORY packaging
                        COMMENT "Building More HQ package...")

   ADD_CUSTOM_COMMAND(TARGET PACKAGE_WIP
                        COMMAND ${NSIS_MAKE_EXE} speed-dreams-wip-cars-and-tracks.nsi
                        WORKING_DIRECTORY packaging
                        COMMENT "Building WIP package...")

   ADD_CUSTOM_COMMAND(TARGET PACKAGE_UNMAINTAINED
                        COMMAND ${NSIS_MAKE_EXE} speed-dreams-unmaintained.nsi
                        WORKING_DIRECTORY packaging
                        COMMENT "Building unmaintained package...")
else(NSIS_FOUND AND NSIS_MAKE_EXE AND NSIS_INET_PLUGIN)
   if(NOT NSIS_FOUND)
      Message(WARNING "NSIS NOT FOUND Packaging targets NOT added.")
   elseif(NOT NSIS_INET_PLUGIN)
      Message(WARNING "INetC.dll NOT FOUND Packaging targets NOT added.")
   endif(NOT NSIS_FOUND)
endif()