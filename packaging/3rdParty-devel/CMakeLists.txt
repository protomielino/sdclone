#
#==============================================================================
#
#    file                 : CMakeLists.txt
#    created              : Aug 5 2015
#    copyright            : (C) 2015-2020 Joe Thompson
#    email                : beaglejoe@users.sourceforge.net
#    version              : $Id$
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
#
# (hashtag) starts a comment
cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(3rdParty-2.3)

include(ExternalProject)
include(CMakeDependentOption)

include(thirdpartydefinitions.cmake)

message(STATUS "CMAKE_VERSION = ${CMAKE_VERSION}")
# Setup some defualts...

# Setup the configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
    message(STATUS "No CMAKE_BUILD_TYPE specified, defaulting to ${CMAKE_BUILD_TYPE}")
endif(NOT CMAKE_BUILD_TYPE)

# Setup the install prefix.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "./installed" CACHE PATH "Prefix prepended to install directories" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


if(MINGW)
   link_directories(/usr/lib /usr/local/lib)
   include_directories(AFTER /usr/include /usr/local/include)
endif(MINGW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(EXTERNALS_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/deps" CACHE PATH "")
set(EXTERNALS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rdParty" CACHE PATH "")
set(EXTERNALS_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNALS_INSTALL_DIR} CACHE PATH "")

set(_max_ext_path "64")
string(LENGTH ${EXTERNALS_PREFIX} _path_length)
if(_path_length GREATER _max_ext_path)
   message(STATUS "The length of the EXTERNALS_PREFIX (${EXTERNALS_PREFIX}) is possibly too long (${_path_length}) to build OSG")
   message(SEND_ERROR "The length of the EXTERNALS_PREFIX needs to be less than ${_max_ext_path} to build OSG")
endif()

set_property(DIRECTORY PROPERTY EP_BASE ${EXTERNALS_PREFIX})

if(WIN32)
   set(OPTION_OPENGL_HEADERS true CACHE BOOL "Download the OpenGL headers")
endif(WIN32)


option(OPTION_SDL_1x "Download and Build SDL 1.x" true)
option(OPTION_SDL_2x "Download and Build SDL 2.x" true)
option(OPTION_OPENAL "Download and Build OpenAL" true)
option(OPTION_PLIB "Download and Build PLIB" true)
cmake_dependent_option(OPTION_PLIB_USE_HEAD "Download and Build PLIB from the latest revision" true "OPTION_PLIB" false)
option(OPTION_JPEG "Download and Build libjpeg" true)
option(OPTION_FREESOLID "Download and Build FreeSOLID" true)
option(OPTION_ENET "Download and Build Enet" true)
option(OPTION_OGG "Download and Build libogg" true)
option(OPTION_VORBIS "Download and Build libvorbis" true)
option(OPTION_EXPAT "Download and Build Expat" true)
option(OPTION_ZLIB "Download and Build zlib" true)
option(OPTION_PNG "Download and Build libpng" true)
option(OPTION_SQLITE3 "Download and Build sqlite3" false)
option(OPTION_FREETYPE "Download and Build FreeType" true)
option(OPTION_CURL "Download and Build curl" true)
option(OPTION_OSG "Download and Build OpenSceneGraph" true)


# CMAKE_BUILD_TYPE does NOT work for Visual Studio or Xcode
# need to switch on configuration somehow
# or limit the choices
if(CMAKE_BUILD_TYPE MATCHES "Debug")
   set(SUPER_BUILD_POSTFIX "d")
else()
   set(SUPER_BUILD_POSTFIX "")
endif()

# OpenGL extensions (Windows only??)
message(STATUS "OpenGL extensions (Windows only??)")
if(WIN32 AND OPTION_OPENGL_HEADERS)
   ExternalProject_Add(OpenGL-headers
      URL ${CMAKE_CURRENT_SOURCE_DIR}/patches/openglheaders.zip
      URL_HASH SHA256=e66ae96912231de021e17c80a978765c126b26cfde1103f63e9f33e4780f7fad
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
endif(WIN32 AND OPTION_OPENGL_HEADERS)

if(OPTION_SDL_1x)
   ExternalProject_Add(${SDL1_PROJECT}
      URL ${SDL_1_URL}
      URL_HASH ${SDL_1_HASH}
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
      DEPENDS OpenGL-headers)
   ExternalProject_Add_Step(${SDL1_PROJECT} sdl1x_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/${SDL1_PROJECT}.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch)  # do before patch
endif(OPTION_SDL_1x)

# TODO - need to disable SDL2 static (it is overwritten by dll's SDL2.lib)
# message(WARNING "TODO - need to disable SDL2 static (it is overwritten by dll's SDL2.lib)")

if(OPTION_SDL_2x)
   set(SDL2_ARGS ${EXTERNALS_CMAKE_ARGS} -DSDL_STATIC=Off -DSDL_CMAKE_DEBUG_POSTFIX=)
   ExternalProject_Add(${SDL2_PROJECT}
      URL ${SDL_2_URL}
      URL_HASH ${SDL_2_HASH}
      CMAKE_ARGS ${SDL2_ARGS}
      DEPENDS OpenGL-headers)
   message(STATUS "Remove the step below when SDL2 CMake is fixed")
   ExternalProject_Add_Step(${SDL2_PROJECT} sdl2_fix_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/sdl-${SDL2_VERSION}.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch)  # do before patch
# end  message(WARNING "Remove the step below when SDL2 CMake is fixed")

else(OPTION_SDL_2x)
   set(SDL2_PROJECT )
endif(OPTION_SDL_2x)

if(OPTION_OPENAL)
   if(MSVC_VERSION LESS 1900)
      message(WARNING "openal-soft versions newer than 1.18.2 need Visual Studio 2015")
      ExternalProject_Add(${OPENAL_LEGACY_PROJECT}
         URL ${OPENAL_LEGACY_URL}
         URL_HASH ${OPENAL_LEGACY_HASH}
         CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
      ExternalProject_Add_Step(${OPENAL_LEGACY_PROJECT} openal_patch_cmake
         COMMAND ${CMAKE_COMMAND} -E copy_if_different
         "${CMAKE_SOURCE_DIR}/patches/${OPENAL_LEGACY_PROJECT}.CMakeLists.txt"
         "<SOURCE_DIR>/CMakeLists.txt"
         DEPENDEES update # do after update
         DEPENDERS patch)  # do before patch
   elseif(MSVC_VERSION LESS 1910)
      message(WARNING "openal-soft versions newer than 1.19.1 need Visual Studio 2017 and CMAKE_SYSTEM_VERSION >= 10")
      ExternalProject_Add(${OPENAL_C11_PROJECT}
         URL ${OPENAL_C11_URL}
         URL_HASH ${OPENAL_C11_HASH}
         CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
   else()
      if(${CMAKE_SYSTEM_VERSION} VERSION_LESS "10.0")
         message(WARNING "Version 1.21.1 of openal-soft is available for Visual Studio 2017 delete the cache and specify -D CMAKE_SYSTEM_VERSION=10.0 on the command line")
         ExternalProject_Add(${OPENAL_C11_PROJECT}
            URL ${OPENAL_C11_URL}
            URL_HASH ${OPENAL_C11_HASH}
            CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
      else()
         ExternalProject_Add(${OPENAL_PROJECT}
            URL ${OPENAL_URL}
            URL_HASH ${OPENAL_HASH}
            CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
      endif()
   endif()
endif(OPTION_OPENAL)

if(OPTION_PLIB)
   if(OPTION_PLIB_USE_HEAD)
      ExternalProject_Add(${PLIB_HEAD_PROJECT}
         SVN_REPOSITORY  ${PLIB_SVN_REPO} #svn://svn.code.sf.net/p/plib/code/trunk
         SVN_REVISION -r${PLIB_SVN_REVISION}  #-r2173 as of 2012-08-24 can also use -rHEAD, but specific revision is supposed to prevent calling svn update
         CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
         DEPENDS OpenGL-headers)
         ExternalProject_Add_Step(${PLIB_HEAD_PROJECT} plib_add_cmake
         COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 "${CMAKE_SOURCE_DIR}/patches/${PLIB_PROJECT}.CMakeLists.txt"
                 "<SOURCE_DIR>/CMakeLists.txt"
         DEPENDEES update # do after update
         DEPENDERS patch)  # do before patch
   else(OPTION_PLIB_USE_HEAD)
      # this tarball corresponds to revision 2131
      ExternalProject_Add(${PLIB_PROJECT}
         URL ${PLIB_URL}
         URL_HASH  ${PLIB_HASH}
         CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
         DEPENDS OpenGL-headers)
      ExternalProject_Add_Step(${PLIB_PROJECT} plib_add_cmake
         COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 "${CMAKE_SOURCE_DIR}/patches/${PLIB_PROJECT}.CMakeLists.txt"
                 "<SOURCE_DIR>/CMakeLists.txt"
         DEPENDEES update # do after update
         DEPENDERS patch)  # do before patch
   endif(OPTION_PLIB_USE_HEAD)
endif(OPTION_PLIB)

if(OPTION_JPEG)
   #set(JPG_PROJECT jpeg-9d)
   ExternalProject_Add(${JPEG_PROJECT}
   URL ${JPEG_URL}
   URL_HASH ${JPEG_HASH}
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
   ExternalProject_Add_Step(${JPEG_PROJECT} jpeg_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/${JPEG_PROJECT}.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch)  # do before patch
   ExternalProject_Add_Step(${JPEG_PROJECT} jpeg_config
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "<SOURCE_DIR>/jconfig.vc"
              "<SOURCE_DIR>/jconfig.h"
      DEPENDEES update # do after update
      DEPENDERS patch)  # do before patch
else(OPTION_JPEG)
   set(JPG_PROJECT )
endif(OPTION_JPEG)

if(OPTION_FREESOLID)
   set(FreeSOLID_ARGS ${EXTERNALS_CMAKE_ARGS} -D'CMAKE_DEBUG_POSTFIX=""')
   ExternalProject_Add(${FREESOLID_PROJECT}
      DOWNLOAD_NAME ${FREESOLID_FILE}
      URL ${FREESOLID_URL}
      URL_HASH ${FREESOLID_HASH}
      CMAKE_ARGS ${FreeSOLID_ARGS})
   ExternalProject_Add_Step(${FREESOLID_PROJECT} freesolid_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/${FREESOLID_PROJECT}.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch)  # do before patch
endif(OPTION_FREESOLID)

if(OPTION_ENET)
   ExternalProject_Add(${ENET_PROJECT}
      URL ${ENET_URL}
      URL_HASH ${ENET_HASH}
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
   ExternalProject_Add_Step(${ENET_PROJECT} enet_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/${ENET_PROJECT}.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch)  # do before patch
else(OPTION_ENET)
   set(ENET_PROJECT )
endif(OPTION_ENET)

if(OPTION_OGG)
   set(OGG_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS=on -DBUILD_TESTING=off -DINSTALL_DOCS=off)
   ExternalProject_Add(${OGG_PROJECT}
      URL ${OGG_URL}
      URL_HASH ${OGG_HASH}
      CMAKE_ARGS ${OGG_ARGS})
else(OPTION_OGG)
   set(OGG_PROJECT )
endif(OPTION_OGG)

if(OPTION_VORBIS)
   set(VORBIS_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS=on)
   ExternalProject_Add(${VORBIS_PROJECT}
      URL ${VORBIS_URL}
      URL_HASH ${VORBIS_HASH}
      CMAKE_ARGS ${VORBIS_ARGS}
      DEPENDS ${OGG_PROJECT})
else(OPTION_VORBIS)
   set(VORBIS_PROJECT )
endif(OPTION_VORBIS)

if(OPTION_EXPAT)
#disable building unneeded sub projects (they failed with VS2008)
   set(EXPAT_ARGS ${EXTERNALS_CMAKE_ARGS} -DEXPAT_BUILD_TESTS=off -DEXPAT_BUILD_EXAMPLES=off -DEXPAT_BUILD_TOOLS=off -DCMAKE_DEBUG_POSTFIX='')
   ExternalProject_Add(${EXPAT_PROJECT}
      DOWNLOAD_NAME ${EXPAT_FILE}
      URL ${EXPAT_URL}
      URL_HASH ${EXPAT_HASH}
      CMAKE_ARGS ${EXPAT_ARGS})
else(OPTION_EXPAT)
   set(EXPAT_PROJECT )
endif(OPTION_EXPAT)

if(OPTION_ZLIB)
   set(ZLIB_PROJECT zlib-1.2.11)
   ExternalProject_Add(${ZLIB_PROJECT}
      DOWNLOAD_NAME ${ZLIB_FILE}
      URL ${ZLIB_URL}
      URL_HASH ${ZLIB_HASH}
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
      PATCH_COMMAND ${CMAKE_COMMAND} -E remove -f <SOURCE_DIR>/zconf.h)
   ExternalProject_Add_Step(${ZLIB_PROJECT} sd_zlib_remove_dll
      COMMAND "${CMAKE_COMMAND}" -E remove -f ${EXTERNALS_INSTALL_DIR}/bin/zlib${SUPER_BUILD_POSTFIX}.dll
      DEPENDEES install)
   ExternalProject_Add_Step(${ZLIB_PROJECT} sd_zlib_remove_lib
      COMMAND "${CMAKE_COMMAND}" -E remove -f ${EXTERNALS_INSTALL_DIR}/lib/zlib${SUPER_BUILD_POSTFIX}.lib
      DEPENDEES sd_zlib_remove_dll)
   ExternalProject_Add_Step(${ZLIB_PROJECT} sd_zlib_rename_lib
      COMMAND "${CMAKE_COMMAND}" -E rename ${EXTERNALS_INSTALL_DIR}/lib/zlibstatic${SUPER_BUILD_POSTFIX}.lib ${EXTERNALS_INSTALL_DIR}/lib/zlib.lib
      DEPENDEES sd_zlib_remove_lib)
else(OPTION_ZLIB)
   set(ZLIB_PROJECT )
endif(OPTION_ZLIB)

if(OPTION_PNG)
   set(PNG_ARGS ${EXTERNALS_CMAKE_ARGS} -DPNG_SHARED=off -DPNG_TESTS=off -DCMAKE_DEBUG_POSTFIX='')
   ExternalProject_Add(${PNG_PROJECT}
      DOWNLOAD_NAME ${PNG_FILE}
      URL ${PNG_URL}
      URL_HASH ${PNG_HASH}
      CMAKE_ARGS ${PNG_ARGS}
      DEPENDS ${ZLIB_PROJECT})
      ExternalProject_Add_Step(${PNG_PROJECT} sd_libpng_rename
         COMMAND "${CMAKE_COMMAND}" -E rename ${EXTERNALS_INSTALL_DIR}/lib/libpng16_static${SUPER_BUILD_POSTFIX}.lib ${EXTERNALS_INSTALL_DIR}/lib/libpng.lib
         DEPENDEES install)
else(OPTION_PNG)
   set(PNG_PROJECT )
endif(OPTION_PNG)

if(OPTION_SQLITE3)
   # may only be needed for Simon's replay stuff
   ExternalProject_Add(${SQLITE3_PROJECT}
      URL ${SQLITE3_URL}
      URL_HASH ${SQLITE3_HASH}
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
   ExternalProject_Add_Step(${SQLITE3_PROJECT} sqlite3_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/sqlite3-CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch)  # do before patch
else(OPTION_SQLITE3)
   set(SQLITE3_PROJECT )
endif(OPTION_SQLITE3)

   # ========== For OpenSceneGraph =============

if(OPTION_FREETYPE)
   set(FREETYPE_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS:BOOL=false
                                             -DDISABLE_FORCE_DEBUG_POSTFIX:BOOL=true
                                             -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE)

   ExternalProject_Add(${FREETYPE_PROJECT}
      URL ${FREETYPE_URL}
      URL_HASH ${FREETYPE_HASH}
      CMAKE_ARGS ${FREETYPE_ARGS}
      DEPENDS ${ZLIB_PROJECT} ${PNG_PROJECT})
else(OPTION_FREETYPE)
   set(FREETYPE_PROJECT )
endif(OPTION_FREETYPE)

if(OPTION_CURL)
   set(CURL_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_TESTING=Off -DCMAKE_DEBUG_POSTFIX=)
   ExternalProject_Add(${CURL_PROJECT}
      URL ${CURL_URL}
      URL_HASH ${CURL_HASH}
      CMAKE_ARGS ${CURL_ARGS}
      DEPENDS ${ZLIB_PROJECT})
else(OPTION_CURL)
   set(CURL_PROJECT )
endif(OPTION_CURL)

if(OPTION_OSG)
   set(OSG_ARGS ${EXTERNALS_CMAKE_ARGS} -DCMAKE_DEBUG_POSTFIX='' -DUSE_3RDPARTY_BIN=Off)

   message(STATUS "Remove the block below when curl fixes their CURLConfig.cmake or OSG fixes it")
   if(NOT ${CMAKE_VERSION} VERSION_LESS "3.17.0")
      list(APPEND OSG_ARGS -DCURL_NO_CURL_CMAKE=ON)
   endif(NOT ${CMAKE_VERSION} VERSION_LESS "3.17.0")
   message(STATUS "OSG_ARGS = ${OSG_ARGS}")

   ExternalProject_Add(${OSG_PROJECT}
      URL ${OSG_URL}
      URL_HASH ${OSG_HASH}
      CMAKE_ARGS ${OSG_ARGS}
      DEPENDS ${ZLIB_PROJECT} ${CURL_PROJECT} ${FREETYPE_PROJECT} OpenGL-headers ${JPG_PROJECT} ${PNG_PROJECT} SDL-1.2.15 ${SDL2_PROJECT})
endif(OPTION_OSG)


# no-output target, for having some files available in IDEs.
set(DOCSOURCES COPYING LICENSE readme.txt)

add_custom_target(rootdoc SOURCES ${DOCSOURCES})

file(GLOB_RECURSE _INFO_FILES "${EXTERNALS_PREFIX}/Stamp/*info.txt")

# ========== Installation =============

install(FILES ${_INFO_FILES} DESTINATION ${EXTERNALS_INSTALL_DIR}/source_info)

install(FILES ${DOCSOURCES} DESTINATION ${EXTERNALS_INSTALL_DIR})

install(DIRECTORY Licenses/ DESTINATION ${EXTERNALS_INSTALL_DIR}/Licenses)

install(DIRECTORY ${EXTERNALS_INSTALL_DIR} DESTINATION .)


# ========== Packaging =============
if(MSVC)
   if(CMAKE_CL_64)
      set(SD_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-win64-${CMAKE_C_COMPILER_ID}-${CMAKE_C_COMPILER_VERSION}-${CMAKE_BUILD_TYPE})
   else(CMAKE_CL_64)
      set(SD_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-win32-${CMAKE_C_COMPILER_ID}-${CMAKE_C_COMPILER_VERSION}-${CMAKE_BUILD_TYPE})
   endif(CMAKE_CL_64)
else(MSVC)
   set(SD_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_C_COMPILER_ID}-${CMAKE_C_COMPILER_VERSION}-${CMAKE_BUILD_TYPE})
endif(MSVC)

set(CPACK_PACKAGE_FILE_NAME ${SD_PACKAGE_FILE_NAME} CACHE STRING "")

set(CPACK_GENERATOR "ZIP")

include(CPack)
