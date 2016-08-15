#
#==============================================================================
#
#    file                 : CMakeLists.txt
#    created              : Aug 5 2015
#    copyright            : (C) 2015 Joe Thompson
#    email                : beaglejoe@users.sourceforge.net
#    version              : $Id: $
#
#==============================================================================
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
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
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(3rdParty-2.3)

include(ExternalProject)
include(CMakeDependentOption)


# Setup some defualts...

# Setup the configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
    message("No CMAKE_BUILD_TYPE specified, defaulting to ${CMAKE_BUILD_TYPE}") 
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
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

set(EXTERNALS_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/externals" CACHE PATH "")
set(EXTERNALS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rdParty" CACHE PATH "")
set(EXTERNALS_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNALS_INSTALL_DIR} CACHE PATH "")

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
   URL_HASH SHA256=4BA471B7306D47776C916BD507B89415BAF6D5662E17A27C2C575D8E331C985B
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
endif(WIN32 AND OPTION_OPENGL_HEADERS)

if(OPTION_SDL_1x)
ExternalProject_Add(SDL-1.2.15 
   URL https://www.libsdl.org/release/SDL-1.2.15.tar.gz 
   URL_HASH SHA256=D6D316A793E5E348155F0DD93B979798933FB98AA1EDEBCC108829D6474AAD00 
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
   set(SDL2_PROJECT SDL2-2.0.4)
   set(SDL2_ARGS ${EXTERNALS_CMAKE_ARGS} -DSDL_STATIC=Off)
   #message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
   ExternalProject_Add(${SDL2_PROJECT} 
   URL https://www.libsdl.org/release/SDL2-2.0.4.tar.gz 
   URL_HASH SHA256=DA55E540BF6331824153805D58B590A29C39D2D506C6D02FA409AEDEAB21174B 
   CMAKE_ARGS ${SDL2_ARGS}
   DEPENDS OpenGL-headers
   )
   message(STATUS "Patching SDL 2.0.4 for DirectInput Haptics...")
   message(STATUS "TODO - check if this patch is still needed with SDL2 > 2.0.4")
   ExternalProject_Add_Step(${SDL2_PROJECT} sdl_2-dinput-patch
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/patches/SDL_dinputhaptic.c.patched"
            "<SOURCE_DIR>/src/haptic/windows/SDL_dinputhaptic.c"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
else(OPTION_SDL_2x)
   set(SDL2_PROJECT )
endif(OPTION_SDL_2x)

if(OPTION_OPENAL)
ExternalProject_Add(openal-soft-1.17.2 
   URL http://kcat.strangesoft.net/openal-releases/openal-soft-1.17.2.tar.bz2 
   URL_HASH SHA256=A341F8542F1F0B8C65241A17DA13D073F18EC06658E1A1606A8ECC8BBC2B3314
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS})
endif(OPTION_OPENAL)

if(OPTION_PLIB)
   if(OPTION_PLIB_USE_HEAD)
   ExternalProject_Add(plib-1.8.5-head 
      SVN_REPOSITORY  svn://svn.code.sf.net/p/plib/code/trunk
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

   ExternalProject_Add(plib-1.8.5 
      URL https://plib.sourceforge.net/dist/plib-1.8.5.tar.gz 
      URL_HASH SHA256=485B22BF6FDC0DA067E34EAD5E26F002B76326F6371E2AE006415DEA6A380A32 
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
   set(JPG_PROJECT jpeg-9b)
   ExternalProject_Add(${JPG_PROJECT}
   URL http://ijg.org/files/jpegsrc.v9b.tar.gz
   URL_HASH SHA256=240FD398DA741669BF3C90366F58452EA59041CACC741A489B99F2F6A0BAD052
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   )
   ExternalProject_Add_Step(${JPG_PROJECT} jpeg_add_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/jpeg-9b.CMakeLists.txt"
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
#   URL_HASH SHA256=5A4F8576E27223F03AD0ADD76A79B06DB6CD8F15C8D2767EA35E0C540BA1BEE8 
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
   URL_HASH SHA256=89EDC6AFDD9D60C8020B2B865B61558C86A8928DC6F1773B9F4708B5C28EB873 
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
ExternalProject_Add(enet-1.3.13
   URL http://enet.bespin.org/download/enet-1.3.13.tar.gz
   URL_HASH SHA256=E36072021FAA28731B08C15B1C3B5B91B911BAF5F6ABCC7FE4A6D425ABADA35C
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   )
   ExternalProject_Add_Step(enet-1.3.13 enet_add_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/enet-1.3.13.CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
endif(OPTION_ENET)

if(OPTION_OGG)
ExternalProject_Add(libogg-1.3.2
   URL http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz
   URL_HASH SHA256=E19EE34711D7AF328CB26287F4137E70630E7261B17CBE3CD41011D73A654692
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   )
   ExternalProject_Add_Step(libogg-1.3.2 libogg_add_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/libogg-1.3.2.CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
endif(OPTION_OGG)

if(OPTION_VORBIS)
ExternalProject_Add(libvorbis-1.3.5
   URL http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.gz
   URL_HASH SHA256=6EFBCECDD3E5DFBF090341B485DA9D176EB250D893E3EB378C428A2DB38301CE
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   DEPENDS "libogg-1.3.2"
   )
   ExternalProject_Add_Step(libvorbis-1.3.5 libvorbis_add_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/libvorbis-1.3.5.CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
endif(OPTION_VORBIS)

#disable building unneeded sub projects (they failed with VS2008)
set(EXPAT_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_tests=off -DBUILD_examples=off -DBUILD_tools=off -DCMAKE_DEBUG_POSTFIX='')
if(OPTION_EXPAT)
   ExternalProject_Add(expat-2.2.0
   DOWNLOAD_NAME expat-2.2.0.tar.bz2
   URL https://sourceforge.net/projects/expat/files/expat/2.2.0/expat-2.2.0.tar.bz2/download
   URL_HASH SHA256=D9E50FF2D19B3538BD2127902A89987474E1A4DB8E43A66A4D1A712AB9A504FF
   CMAKE_ARGS ${EXPAT_ARGS}
   #UPDATE_COMMAND ""
   #PATCH_COMMAND ""
   #CONFIGURE_COMMAND ""
   #BUILD_COMMAND ""
   #INSTALL_COMMAND ""  
   )
   # Hopefully temporary - report to Expat
   # https://sourceforge.net/p/expat/bugs/534/
   message(STATUS "Todo - remove after Expat's bug 534 gets fixed...")
   ExternalProject_Add_Step(expat-2.2.0 expat-2.2.0_fix_cmake
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/expat-2.2.0-CMakeLists.txt"
           "<SOURCE_DIR>/CMakeLists.txt"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
   )
endif(OPTION_EXPAT)

message(STATUS "CMAKE_COMMAND = ${CMAKE_COMMAND}")
#set(ZLIB_ARGS ${EXTERNALS_CMAKE_ARGS} -DSKIP_INSTALL_LIBRARIES=On -DSKIP_INSTALL_LIBRARIES=On -D'CMAKE_DEBUG_POSTFIX=""')
#set(ZLIB_ARGS ${EXTERNALS_CMAKE_ARGS} -D'CMAKE_BUILD_POSTFIX=hello')

if(OPTION_ZLIB)
ExternalProject_Add(zlib-1.2.8
   DOWNLOAD_NAME zlib-1.2.8.tar.gz
   URL https://sourceforge.net/projects/libpng/files/zlib/1.2.8/zlib-1.2.8.tar.gz/download
   URL_HASH SHA256=36658CB768A54C1D4DEC43C3116C27ED893E88B02ECFCB44F2166F9C0B7F2A0D
   #CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS:BOOL=on
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   #UPDATE_COMMAND ""
   PATCH_COMMAND ${CMAKE_COMMAND} -E remove -f <SOURCE_DIR>/zconf.h
   #CONFIGURE_COMMAND ""
   #BUILD_COMMAND ""
   #INSTALL_COMMAND ${CMAKE_COMMAND} install(TARGETS zlibstatic)
   )
ExternalProject_Add_Step(zlib-1.2.8 sd_zlib_remove_dll
   COMMAND "${CMAKE_COMMAND}" -E remove -f ${EXTERNALS_INSTALL_DIR}/bin/zlib${SUPER_BUILD_POSTFIX}.dll 
   DEPENDEES install
   )
ExternalProject_Add_Step(zlib-1.2.8 sd_zlib_remove_lib
   COMMAND "${CMAKE_COMMAND}" -E remove -f ${EXTERNALS_INSTALL_DIR}/lib/zlib${SUPER_BUILD_POSTFIX}.lib
   DEPENDEES sd_zlib_remove_dll
   )
ExternalProject_Add_Step(zlib-1.2.8 sd_zlib_rename_lib
   COMMAND "${CMAKE_COMMAND}" -E rename ${EXTERNALS_INSTALL_DIR}/lib/zlibstatic${SUPER_BUILD_POSTFIX}.lib ${EXTERNALS_INSTALL_DIR}/lib/zlib.lib 
   DEPENDEES sd_zlib_remove_lib
   )
endif(OPTION_ZLIB)

if(OPTION_PNG)
   set(PNG_PROJECT libpng-1.6.24)
   set(PNG_ARGS ${EXTERNALS_CMAKE_ARGS} -DPNG_SHARED=off -DPNG_TESTS=off -DCMAKE_DEBUG_POSTFIX='')
   ExternalProject_Add(${PNG_PROJECT} 
      DOWNLOAD_NAME ${PNG_PROJECT}.tar.gz 
      URL https://sourceforge.net/projects/libpng/files/libpng16/1.6.24/libpng-1.6.24.tar.gz/download 
      URL_HASH MD5=65213080DD30A9B16193D9B83ADC1EE9
      CMAKE_ARGS ${PNG_ARGS}
      DEPENDS zlib-1.2.8
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
   ExternalProject_Add(sqlite3
   URL https://www.sqlite.org/2015/sqlite-autoconf-3081101.tar.gz
   URL_HASH SHA1=D0E22D7E361B6F50830A3CDEAFE35311443F8F9A 
   CMAKE_ARGS ${EXTERNALS_CMAKE_ARGS}
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   CONFIGURE_COMMAND ""
   BUILD_COMMAND ""
   INSTALL_COMMAND ""  
   )
endif(OPTION_SQLITE3)

   # ========== For OpenSceneGraph =============

if(OPTION_FREETYPE)
   set(FREETYPE_PROJECT freetype-2.6.5)
   set(FREETYPE_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_SHARED_LIBS:BOOL=false -DWITH_PNG=OFF)

   ExternalProject_Add(${FREETYPE_PROJECT}
   DOWNLOAD_NAME ft265.zip
   URL https://sourceforge.net/projects/freetype/files/freetype2/2.6.5/ft265.zip/download
   URL_HASH SHA256=635309058FF8B8056BB94488FFC26CC984ED632E01DDFE80E4C0768930483182 
   CMAKE_ARGS ${FREETYPE_ARGS}
   )

   # Rename the lib to remove 'd' post-fix for debug builds
   ExternalProject_Add_Step(${FREETYPE_PROJECT} freetypelib_rename
   COMMAND "${CMAKE_COMMAND}" -E rename ${EXTERNALS_INSTALL_DIR}/lib/freetype${SUPER_BUILD_POSTFIX}.lib ${EXTERNALS_INSTALL_DIR}/lib/freetype.lib
   DEPENDEES install
   )

else(OPTION_FREETYPE)
   set(FREETYPE_PROJECT )
endif(OPTION_FREETYPE)

if(OPTION_CURL)
#   set(CURL_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_CURL_TESTS=Off)
#   # may only be needed for Madbad's web stuff
#   ExternalProject_Add(curl-7.43.0
#   URL https://curl.haxx.se/download/curl-7.43.0.tar.bz2
#   URL_HASH SHA256=BAA654A1122530483CCC1C58CC112FEC3724A82C11C6A389F1E6A37DC8858DF9
#   CMAKE_ARGS ${CURL_ARGS}
#   DEPENDS zlib-1.2.8
#   )
#   
   set(CURL_PROJECT curl-7.50.1)
   set(CURL_ARGS ${EXTERNALS_CMAKE_ARGS} -DBUILD_TESTING=Off)
   # may only be needed for Madbad's web stuff
   ExternalProject_Add(${CURL_PROJECT}
   URL https://curl.haxx.se/download/curl-7.50.1.tar.bz2
   URL_HASH SHA256=3C12C5F54CCAA1D40ABC65D672107DCC75D3E1FCB38C267484334280096E5156
   CMAKE_ARGS ${CURL_ARGS}
   DEPENDS zlib-1.2.8
   )
else(OPTION_CURL)
   set(CURL_PROJECT )
endif(OPTION_CURL)

if(OPTION_OSG)
   set(OSG_ARGS ${EXTERNALS_CMAKE_ARGS} -DCMAKE_DEBUG_POSTFIX='')

   # http://trac.openscenegraph.org/downloads/developer_releases/OpenSceneGraph-3.4.0.zip
   # A5E762C64373A46932E444F6F7332496
   #ExternalProject_Add(osg-OpenSceneGraph-3.4.0-rc10
   #URL https://github.com/openscenegraph/osg/archive/OpenSceneGraph-3.4.0-rc10.tar.gz
   #URL_HASH SHA256=47D4281E86B3B3EE14E40A46FF60884ED2213E9D41204BE76A1F57A6E01CBB0D

   ExternalProject_Add(osg-OpenSceneGraph-3.4.0
   URL http://trac.openscenegraph.org/downloads/developer_releases/OpenSceneGraph-3.4.0.zip
   URL_HASH MD5=A5E762C64373A46932E444F6F7332496
   CMAKE_ARGS ${OSG_ARGS}
   DEPENDS zlib-1.2.8 ${CURL_PROJECT} ${FREETYPE_PROJECT} OpenGL-headers ${JPG_PROJECT} ${PNG_PROJECT} SDL-1.2.15 ${SDL2_PROJECT}
   )

   message(STATUS "Todo - remove after OSG > 3.4.0 as this fix is already merged upstream.")
   ExternalProject_Add_Step(osg-OpenSceneGraph-3.4.0 osg_fix_types
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
           "${CMAKE_SOURCE_DIR}/patches/osg-3.4.0.types.txt"
           "<SOURCE_DIR>/include/osg/Types"
   DEPENDEES update # do after update
   DEPENDERS patch  # do before patch
)
endif(OPTION_OSG)

#add_subdirectory(src)

# no-output target, for having some files available in IDEs.
set(DOCSOURCES COPYING LICENSE readme.txt)

add_custom_target(rootdoc SOURCES ${DOCSOURCES})

install(DIRECTORY ${EXTERNALS_INSTALL_DIR} DESTINATION .)
install(FILES ${DOCSOURCES} DESTINATION 3rdParty)


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


