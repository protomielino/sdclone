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
ExternalProject_Add(SDL-1.2.15
   URL https://www.libsdl.org/release/SDL-1.2.15.tar.gz
   URL_HASH SHA256=d6d316a793e5e348155f0dd93b979798933fb98aa1edebcc108829d6474aad00
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   DEPENDS OpenGL-headers
   )
ExternalProject_Add_Step(SDL-1.2.15 sdl1x_add_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/sdl-1.2.15.CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
)
endif(OPTION_SDL_1x)

# TODO - need to disable SDL2 static (it is overwritten by dll's SDL2.lib)
# message(WARNING "TODO - need to disable SDL2 static (it is overwritten by dll's SDL2.lib)")

if(OPTION_SDL_2x)
   set(SDL2_PROJECT SDL2-2.0.12)
   set(SDL2_ARGS ${EXTERNALS_CMAKE_ARGS} -DSDL_STATIC=Off -DSDL_CMAKE_DEBUG_POSTFIX=)
#   set(SDL2_ARGS ${EXTERNALS_CMAKE_ARGS} -DSDL_STATIC=Off)
   ExternalProject_Add(${SDL2_PROJECT}
      URL https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
   URL_HASH SHA256=349268f695c02efbc9b9148a70b85e58cefbbf704abd3e91be654db7f1e2c863
   CMAKE_ARGS ${SDL2_ARGS}
   DEPENDS OpenGL-headers
   )
   message(STATUS "Remove the step below when SDL2 CMake is fixed")
   ExternalProject_Add_Step(${SDL2_PROJECT} sdl2_fix_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/sdl-2.0.12.CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
# end  message(WARNING "Remove the step below when SDL2 CMake is fixed")

else(OPTION_SDL_2x)
   set(SDL2_PROJECT )
endif(OPTION_SDL_2x)

if(OPTION_OPENAL)
   if(MSVC_VERSION LESS 1900)
      message("openal-soft versions newer than 1.18.2 need Visual Studio 2015")
      ExternalProject_Add(openal-soft-1.18.2
         URL http://www.openal-soft.org/openal-releases/openal-soft-1.18.2.tar.bz2
         URL_HASH SHA256=9f8ac1e27fba15a59758a13f0c7f6540a0605b6c3a691def9d420570506d7e82
         CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
      ExternalProject_Add_Step(openal-soft-1.18.2 openal_patch_cmake
         COMMAND ${CMAKE_COMMAND} -E copy_if_different
         "${CMAKE_SOURCE_DIR}/patches/openal-soft-1.18.2.CMakeLists.txt"
         "<SOURCE_DIR>/CMakeLists.txt"
         DEPENDEES update # do after update
         DEPENDERS patch  # do before patch
         )
   else(MSVC_VERSION LESS 1900)
      ExternalProject_Add(openal-soft-1.19.1
         URL http://www.openal-soft.org/openal-releases/openal-soft-1.19.1.tar.bz2
         URL_HASH SHA256=5c2f87ff5188b95e0dc4769719a9d89ce435b8322b4478b95dd4b427fe84b2e9
         CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
   endif(MSVC_VERSION LESS 1900)
endif(OPTION_OPENAL)

if(OPTION_PLIB)
   if(OPTION_PLIB_USE_HEAD)
   ExternalProject_Add(plib-1.8.5-head
      SVN_REPOSITORY  svn://svn.code.sf.net/p/plib/code/trunk
      SVN_REVISION -r2173  #-r2173 as of 2012-08-24 can also use -rHEAD, but specific revision is supposed to prevent calling svn update
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
      DEPENDS OpenGL-headers
      )
      ExternalProject_Add_Step(plib-1.8.5-head plib_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/plib-1.8.5.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch  # do before patch
      )

   else(OPTION_PLIB_USE_HEAD)
   # this tarball corresponds to revision 2131
   ExternalProject_Add(plib-1.8.5
      URL http://plib.sourceforge.net/dist/plib-1.8.5.tar.gz
      URL_HASH SHA256=485b22bf6fdc0da067e34ead5e26f002b76326f6371e2ae006415dea6a380a32
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
      DEPENDS OpenGL-headers
      )
      ExternalProject_Add_Step(plib-1.8.5 plib_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/plib-1.8.5.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch  # do before patch
      )
   endif(OPTION_PLIB_USE_HEAD)
endif(OPTION_PLIB)

if(OPTION_JPEG)
   set(JPG_PROJECT jpeg-9d)
   ExternalProject_Add(${JPG_PROJECT}
   URL http://ijg.org/files/jpegsrc.v9d.tar.gz
   URL_HASH SHA256=99cb50e48a4556bc571dadd27931955ff458aae32f68c4d9c39d624693f69c32
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   )
   ExternalProject_Add_Step(${JPG_PROJECT} jpeg_add_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/jpeg-9d.CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
   ExternalProject_Add_Step(${JPG_PROJECT} jpeg_config
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "<SOURCE_DIR>/jconfig.vc"
           "<SOURCE_DIR>/jconfig.h"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
else(OPTION_JPEG)
   set(JPG_PROJECT )
endif(OPTION_JPEG)

if(OPTION_FREESOLID)
#message(WARNING "TODO - need to decide between the patches for FreeSOLID-2.1.1 OR FreeSOLID-2.1.2")
#ExternalProject_Add(FreeSOLID-2.1.1 
#   DOWNLOAD_NAME FreeSOLID-2.1.1.zip
#   URL https://sourceforge.net/projects/freesolid/files/FreeSOLID/FreeSOLID-2.1.1.zip/FreeSOLID-2.1.1.zip/download
#   URL_HASH SHA256=5a4f8576e27223f03ad0add76a79b06db6cd8f15c8d2767ea35e0c540ba1bee8 
#   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
#   UPDATE_COMMAND ""
#   PATCH_COMMAND ""
#   CONFIGURE_COMMAND ""
#   BUILD_COMMAND ""
#   INSTALL_COMMAND ""
#   )

set(FreeSOLID_ARGS ${EXTERNALS_CMAKE_ARGS} -D'CMAKE_DEBUG_POSTFIX=""')
ExternalProject_Add(FreeSOLID-2.1.2
   DOWNLOAD_NAME FreeSOLID-2.1.2.zip
   URL https://sourceforge.net/projects/freesolid/files/FreeSOLID-2.1.2.zip/download
   URL_HASH SHA256=89edc6afdd9d60c8020b2b865b61558c86a8928dc6f1773b9f4708b5c28eb873
   CMAKE_ARGS ${FreeSOLID_ARGS}
   )
ExternalProject_Add_Step(FreeSOLID-2.1.2 freesolid_add_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/FreeSOLID-2.1.2.CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
endif(OPTION_FREESOLID)

if(OPTION_ENET)
set(ENET_PROJECT enet-1.3.15)
   ExternalProject_Add(${ENET_PROJECT}
      URL http://enet.bespin.org/download/enet-1.3.15.tar.gz
      URL_HASH SHA256=5abdf63346e54272344d8184b5a2f333d202d809d28123911cbd993e5772bdfb
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   )
   ExternalProject_Add_Step(${ENET_PROJECT} enet_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${CMAKE_SOURCE_DIR}/patches/enet-1.3.15.CMakeLists.txt"
              "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch  # do before patch
   )
else(OPTION_ENET)
   set(ENET_PROJECT )
endif(OPTION_ENET)

if(OPTION_OGG)
   set(OGG_PROJECT ogg-1.3.4)
   set(OGG_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS=on -DBUILD_TESTING=off -DINSTALL_DOCS=off)
   ExternalProject_Add(${OGG_PROJECT}
      URL http://downloads.xiph.org/releases/ogg/libogg-1.3.4.tar.gz
      URL_HASH SHA256=fe5670640bd49e828d64d2879c31cb4dde9758681bb664f9bdbf159a01b0c76e
      CMAKE_ARGS ${OGG_ARGS}
      )
else(OPTION_OGG)
   set(OGG_PROJECT )
endif(OPTION_OGG)

if(OPTION_VORBIS)
   set(VORBIS_PROJECT vorbis-1.3.7)
   set(VORBIS_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS=on)
   ExternalProject_Add(${VORBIS_PROJECT}
      URL http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz
      URL_HASH SHA256=0e982409a9c3fc82ee06e08205b1355e5c6aa4c36bca58146ef399621b0ce5ab
      CMAKE_ARGS ${VORBIS_ARGS}
      DEPENDS ${OGG_PROJECT}
      )
else(OPTION_VORBIS)
   set(VORBIS_PROJECT )
endif(OPTION_VORBIS)

if(OPTION_EXPAT)
   set(EXPAT_PROJECT expat-2.2.10)
#disable building unneeded sub projects (they failed with VS2008)
set(EXPAT_ARGS ${EXTERNALS_CMAKE_ARGS} -DEXPAT_BUILD_TESTS=off -DEXPAT_BUILD_EXAMPLES=off -DEXPAT_BUILD_TOOLS=off -DCMAKE_DEBUG_POSTFIX='')
ExternalProject_Add(${EXPAT_PROJECT}
   DOWNLOAD_NAME ${EXPAT_PROJECT}.tar.bz2
   URL https://sourceforge.net/projects/expat/files/expat/2.2.10/expat-2.2.10.tar.bz2/download
   URL_HASH SHA256=b2c160f1b60e92da69de8e12333096aeb0c3bf692d41c60794de278af72135a5
   CMAKE_ARGS ${EXPAT_ARGS}
   #UPDATE_COMMAND ""
   #PATCH_COMMAND ""
   #CONFIGURE_COMMAND ""
   #BUILD_COMMAND ""
   #INSTALL_COMMAND ""
   )
else(OPTION_EXPAT)
   set(EXPAT_PROJECT )
endif(OPTION_EXPAT)

if(OPTION_ZLIB)
   set(ZLIB_PROJECT zlib-1.2.11)
   #set(ZLIB_ARGS ${EXTERNALS_CMAKE_ARGS} -DSKIP_INSTALL_LIBRARIES=On -DSKIP_INSTALL_LIBRARIES=On -D'CMAKE_DEBUG_POSTFIX=""')
   #set(ZLIB_ARGS ${EXTERNALS_CMAKE_ARGS} -D'CMAKE_BUILD_POSTFIX=hello')
   ExternalProject_Add(${ZLIB_PROJECT}
   DOWNLOAD_NAME ${ZLIB_PROJECT}.tar.gz
   URL https://sourceforge.net/projects/libpng/files/zlib/1.2.11/zlib-1.2.11.tar.gz/download
   URL_HASH SHA256=c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1
   #CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS:BOOL=on
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   #UPDATE_COMMAND ""
   PATCH_COMMAND ${CMAKE_COMMAND} -E remove -f <SOURCE_DIR>/zconf.h
   #CONFIGURE_COMMAND ""
   #BUILD_COMMAND ""
   #INSTALL_COMMAND ${CMAKE_COMMAND} install(TARGETS zlibstatic)
   )
ExternalProject_Add_Step(${ZLIB_PROJECT} sd_zlib_remove_dll
   COMMAND "${CMAKE_COMMAND}" -E remove -f ${EXTERNALS_INSTALL_DIR}/bin/zlib${SUPER_BUILD_POSTFIX}.dll
   DEPENDEES install
   )
ExternalProject_Add_Step(${ZLIB_PROJECT} sd_zlib_remove_lib
   COMMAND "${CMAKE_COMMAND}" -E remove -f ${EXTERNALS_INSTALL_DIR}/lib/zlib${SUPER_BUILD_POSTFIX}.lib
   DEPENDEES sd_zlib_remove_dll
   )
ExternalProject_Add_Step(${ZLIB_PROJECT} sd_zlib_rename_lib
   COMMAND "${CMAKE_COMMAND}" -E rename ${EXTERNALS_INSTALL_DIR}/lib/zlibstatic${SUPER_BUILD_POSTFIX}.lib ${EXTERNALS_INSTALL_DIR}/lib/zlib.lib
   DEPENDEES sd_zlib_remove_lib
   )
else(OPTION_ZLIB)
   set(ZLIB_PROJECT )
endif(OPTION_ZLIB)

if(OPTION_PNG)
   set(PNG_PROJECT libpng-1.6.37)
   set(PNG_ARGS ${EXTERNALS_CMAKE_ARGS} -DPNG_SHARED=off -DPNG_TESTS=off -DCMAKE_DEBUG_POSTFIX='')
   ExternalProject_Add(${PNG_PROJECT}
      DOWNLOAD_NAME ${PNG_PROJECT}.tar.gz
      URL https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/libpng-1.6.37.tar.gz/download
      URL_HASH SHA256=daeb2620d829575513e35fecc83f0d3791a620b9b93d800b763542ece9390fb4
      CMAKE_ARGS ${PNG_ARGS}
      DEPENDS ${ZLIB_PROJECT}
      )
      ExternalProject_Add_Step(${PNG_PROJECT} sd_libpng_rename
      COMMAND "${CMAKE_COMMAND}" -E rename ${EXTERNALS_INSTALL_DIR}/lib/libpng16_static${SUPER_BUILD_POSTFIX}.lib ${EXTERNALS_INSTALL_DIR}/lib/libpng.lib
      DEPENDEES install
      )
else(OPTION_PNG)
   set(PNG_PROJECT )
endif(OPTION_PNG)

if(OPTION_SQLITE3)
   # may only be needed for Simon's replay stuff
      ExternalProject_Add(sqlite3-3.30.1
      URL https://www.sqlite.org/2019/sqlite-amalgamation-3300100.zip
      URL_HASH SHA256=adf051d4c10781ea5cfabbbc4a2577b6ceca68590d23b58b8260a8e24cc5f081
      CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
      #UPDATE_COMMAND ""
      #PATCH_COMMAND ""
      #CONFIGURE_COMMAND ""
      #BUILD_COMMAND ""
      #INSTALL_COMMAND ""
   )
   ExternalProject_Add_Step(sqlite3-3.30.1 sqlite3_add_cmake
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/sqlite3-CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
      DEPENDEES update # do after update
      DEPENDERS patch  # do before patch
   )
endif(OPTION_SQLITE3)

   # ========== For OpenSceneGraph =============

if(OPTION_FREETYPE)
   set(FREETYPE_PROJECT freetype-2.10.2)
   set(FREETYPE_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS:BOOL=false
                                             -DDISABLE_FORCE_DEBUG_POSTFIX:BOOL=true
                                             -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=TRUE
                                             -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE)

   ExternalProject_Add(${FREETYPE_PROJECT}
      URL https://sourceforge.net/projects/freetype/files/freetype2/2.10.2/ft2102.zip/download
      URL_HASH SHA256=d21449e30546554c975cd8ca6897d8ef1717b4c358987ddfb5bb35ebe0be2834
      CMAKE_ARGS ${FREETYPE_ARGS}
      DEPENDS ${ZLIB_PROJECT} ${PNG_PROJECT}
   )
else(OPTION_FREETYPE)
   set(FREETYPE_PROJECT )
endif(OPTION_FREETYPE)

if(OPTION_CURL)
   set(CURL_PROJECT curl-7.72.0)
   set(CURL_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_TESTING=Off -DCMAKE_DEBUG_POSTFIX=)
   ExternalProject_Add(${CURL_PROJECT}
      URL https://curl.haxx.se/download/curl-7.72.0.tar.bz2
      URL_HASH SHA256=ad91970864102a59765e20ce16216efc9d6ad381471f7accceceab7d905703ef
      CMAKE_ARGS ${CURL_ARGS}
      DEPENDS ${ZLIB_PROJECT}
   )
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

#   if(MSVC_VERSION LESS 1900)
#      message("Disable Targa plugin, OSG newer than 3.6.3 need Visual Studio 2015")
#      set(OSG_ARGS ${OSG_ARGS} -D BUILD_OSG_PLUGIN_TGA=0)
#   endif()

   ExternalProject_Add(OpenSceneGraph-3.6.5
      URL https://github.com/openscenegraph/OpenSceneGraph/archive/OpenSceneGraph-3.6.5.zip
      URL_HASH SHA256=0e9e3e4cc6f463f21a901934a95e9264b231a1d5db90f72dcb4b8cc94b0d1b3b
      CMAKE_ARGS ${OSG_ARGS}
      DEPENDS ${ZLIB_PROJECT} ${CURL_PROJECT} ${FREETYPE_PROJECT} OpenGL-headers ${JPG_PROJECT} ${PNG_PROJECT} SDL-1.2.15 ${SDL2_PROJECT}
   )
endif(OPTION_OSG)

#add_subdirectory(src)

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


