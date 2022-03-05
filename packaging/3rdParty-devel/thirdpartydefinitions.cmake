#
#==============================================================================
#
#    file                 : thirdpartydefinitions.cmake
#    created              : June 22 2020
#    copyright            : (C) 2020 Joe Thompson
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
#
#           NOTICE
# When updating OpenSceneGraph, libPNG, or jpeg
# Check /cmake/customthirdparty.cmake
#  The macros 
#     MACRO(SD_INSTALL_CUSTOM_3RDPARTY TARGET_NAME)
#     MACRO(_FIND_3RDPARTY_DEPENDENCIES ROOT_DIR)
#  may need to be updated for the new version


# SDL2
set(SDL2_VERSION 2.0.20)
set(SDL2_PROJECT SDL2-${SDL2_VERSION})
set(SDL2_FILE ${SDL2_PROJECT}.tar.gz)
set(SDL2_URL https://www.libsdl.org/release/${SDL2_FILE})
set(SDL2_HASH SHA256=c56aba1d7b5b0e7e999e4a7698c70b63a3394ff9704b5f6e1c57e0c16f04dd06)

# SDL2_MIXER
set(SDL2_MIXER_VERSION 2.0.4)
set(SDL2_MIXER_PROJECT SDL2_mixer-${SDL2_MIXER_VERSION})
set(SDL2_MIXER_FILE ${SDL2_MIXER_PROJECT}.tar.gz)
set(SDL2_MIXER_URL https://www.libsdl.org/projects/SDL_mixer/release/${SDL2_MIXER_FILE})
set(SDL2_MIXER_HASH SHA256=b4cf5a382c061cd75081cf246c2aa2f9df8db04bdda8dcdc6b6cca55bede2419)

# OpenAL-soft
set(OPENAL_PROJECT_VERSION 1.21.1)
set(OPENAL_PROJECT openal-soft-${OPENAL_PROJECT_VERSION})
set(OPENAL_FILE ${OPENAL_PROJECT}.tar.bz2)
set(OPENAL_URL https://www.openal-soft.org/openal-releases/${OPENAL_FILE})
set(OPENAL_HASH SHA256=c8ad767e9a3230df66756a21cc8ebf218a9d47288f2514014832204e666af5d8)

message(WARNING "openal-soft versions newer than 1.19.1 need Visual Studio 2019 and CMAKE_SYSTEM_VERSION >= 10")
set(OPENAL_C11_VERSION 1.19.1)
set(OPENAL_C11_PROJECT openal-soft-${OPENAL_C11_VERSION})
set(OPENAL_C11_FILE ${OPENAL_C11_PROJECT}.tar.bz2)
set(OPENAL_C11_URL https://www.openal-soft.org/openal-releases/${OPENAL_C11_FILE})
set(OPENAL_C11_HASH SHA256=5c2f87ff5188b95e0dc4769719a9d89ce435b8322b4478b95dd4b427fe84b2e9)

message(STATUS "openal-soft versions newer than 1.18.2 need Visual Studio 2015 or newer")
message(STATUS "so for older versions of Visual Studio, also download this version")
set(OPENAL_LEGACY_VERSION 1.18.2)
set(OPENAL_LEGACY_PROJECT openal-soft-${OPENAL_LEGACY_VERSION})
set(OPENAL_LEGACY_FILE ${OPENAL_LEGACY_PROJECT}.tar.bz2)
set(OPENAL_LEGACY_URL https://www.openal-soft.org/openal-releases/${OPENAL_LEGACY_FILE})
set(OPENAL_LEGACY_HASH SHA256=9f8ac1e27fba15a59758a13f0c7f6540a0605b6c3a691def9d420570506d7e82)

# PLIB
message(STATUS "TODO: Need special handling for PLIB")
message(STATUS "TODO: Switch to the zip file once a download location is setup")
message(STATUS "NOTE: PLIB_HEAD_HASH changes every time a  new zip is generated")
set(PLIB_SVN_REPO svn://svn.code.sf.net/p/plib/code/trunk)
set(PLIB_SVN_REVISION 2173)
set(PLIB_HEAD_VERSION r${PLIB_SVN_REVISION})
set(PLIB_HEAD_PROJECT plib-trunk-${PLIB_HEAD_VERSION})
set(PLIB_HEAD_FILE plib-code-${PLIB_HEAD_VERSION}-trunk.zip)
set(PLIB_HEAD_URL https://sourceforge.net/code-snapshots/svn/p/pl/plib/code/${PLIB_HEAD_FILE})
set(PLIB_HEAD_HASH SHA256=395d27e3182d1e6b4f6c79bce1182040538e07dcc5357b8d7e96f10c7b77f347)

set(PLIB_VERSION 1.8.5)
set(PLIB_PROJECT plib-${PLIB_VERSION})
set(PLIB_FILE ${PLIB_PROJECT}.tar.gz)
set(PLIB_URL http://plib.sourceforge.net/dist/${PLIB_FILE})
set(PLIB_HASH SHA256=485b22bf6fdc0da067e34ead5e26f002b76326f6371e2ae006415dea6a380a32)

# jpeg
set(JPEG_VERSION 9e)
set(JPEG_PROJECT jpeg-${JPEG_VERSION})
set(JPEG_FILE jpegsrc.v${JPEG_VERSION}.tar.gz)
set(JPEG_URL https://ijg.org/files/${JPEG_FILE})
set(JPEG_HASH SHA256=4077d6a6a75aeb01884f708919d25934c93305e49f7e3f36db9129320e6f4f3d)

# freeSOLID
set(FREESOLID_VERSION 2.1.2)
set(FREESOLID_PROJECT FreeSOLID-${FREESOLID_VERSION})
set(FREESOLID_FILE ${FREESOLID_PROJECT}.zip)
set(FREESOLID_URL https://sourceforge.net/projects/freesolid/files/${FREESOLID_FILE}/download)
set(FREESOLID_HASH SHA256=89edc6afdd9d60c8020b2b865b61558c86a8928dc6f1773b9f4708b5c28eb873)

# enet
set(ENET_VERSION 1.3.17)
set(ENET_PROJECT enet-${ENET_VERSION})
set(ENET_FILE ${ENET_PROJECT}.tar.gz)
set(ENET_URL http://enet.bespin.org/download/${ENET_FILE})
set(ENET_HASH SHA256=a38f0f194555d558533b8b15c0c478e946310022d0ec7b34334e19e4574dcedc)

# ogg
set(OGG_VERSION 1.3.5)
set(OGG_PROJECT ogg-${OGG_VERSION})
set(OGG_FILE lib${OGG_PROJECT}.tar.gz)
set(OGG_URL http://downloads.xiph.org/releases/ogg/${OGG_FILE})
set(OGG_HASH SHA256=0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664)

# vorbis
set(VORBIS_VERSION 1.3.7)
set(VORBIS_PROJECT vorbis-${VORBIS_VERSION})
set(VORBIS_FILE lib${VORBIS_PROJECT}.tar.gz)
set(VORBIS_URL http://downloads.xiph.org/releases/vorbis/${VORBIS_FILE})
set(VORBIS_HASH SHA256=0e982409a9c3fc82ee06e08205b1355e5c6aa4c36bca58146ef399621b0ce5ab)

# expat
set(EXPAT_VERSION 2.4.7)
string(REPLACE "." "_" EXPAT_TAG ${EXPAT_VERSION})
set(EXPAT_PROJECT expat-${EXPAT_VERSION})
set(EXPAT_FILE ${EXPAT_PROJECT}.tar.bz2)
set(EXPAT_URL https://github.com/libexpat/libexpat/releases/download/R_${EXPAT_TAG}/${EXPAT_FILE})
set(EXPAT_HASH SHA256=e149bdd8b90254c62b3d195da53a09bd531a4d63a963b0d8a5268d48dd2f6a65)

set(EXPAT_LEGACY_VERSION 2.2.10)
string(REPLACE "." "_" EXPAT_LEGACY_TAG ${EXPAT_LEGACY_VERSION})
set(EXPAT_LEGACY_PROJECT expat-${EXPAT_LEGACY_VERSION})
set(EXPAT_LEGACY_FILE ${EXPAT_LEGACY_PROJECT}.tar.bz2)
set(EXPAT_LEGACY_URL https://github.com/libexpat/libexpat/releases/download/R_${EXPAT_LEGACY_TAG}/${EXPAT_LEGACY_FILE})
set(EXPAT_LEGACY_HASH SHA256=b2c160f1b60e92da69de8e12333096aeb0c3bf692d41c60794de278af72135a5)

# zlib
message(STATUS "Note special path handling (version in path)")
set(ZLIB_VERSION 1.2.11)
set(ZLIB_PROJECT zlib-${ZLIB_VERSION})
set(ZLIB_FILE ${ZLIB_PROJECT}.tar.gz)
set(ZLIB_URL https://sourceforge.net/projects/libpng/files/zlib/${ZLIB_VERSION}/${ZLIB_FILE}/download)
set(ZLIB_HASH SHA256=c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1)

# libpng
message(STATUS "Note special path handling (version in path) AND hard-coded 'libpng16'")
set(PNG_VERSION 1.6.37)
set(PNG_PROJECT libpng-${PNG_VERSION})
set(PNG_FILE ${PNG_PROJECT}.tar.gz)
set(PNG_URL https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/${PNG_FILE}/download)
set(PNG_HASH SHA256=daeb2620d829575513e35fecc83f0d3791a620b9b93d800b763542ece9390fb4)

# freetype
message(STATUS "Note special path handling (version in path) AND hard-coded 'freetype2'")
set(FREETYPE_VERSION 2.11.1)
set(FREETYPE_PROJECT freetype-${FREETYPE_VERSION})
string(REPLACE "." "" FREETYPE_TAG ${FREETYPE_VERSION})
set(FREETYPE_FILE ft${FREETYPE_TAG}.zip)
set(FREETYPE_URL https://sourceforge.net/projects/freetype/files/freetype2/${FREETYPE_VERSION}/${FREETYPE_FILE}/download)
set(FREETYPE_HASH SHA256=9cf52488ebad684d6c967ea377e4a4c0e44454a97c8a0afbf33bba27256dc69c)

message(STATUS "Note special path handling (version in path) AND hard-coded 'freetype2'")
set(FREETYPE_LEGACY_VERSION 2.10.4)
set(FREETYPE_LEGACY_PROJECT freetype-${FREETYPE_LEGACY_VERSION})
set(FREETYPE_LEGACY_FILE ft2104.zip)
set(FREETYPE_LEGACY_URL https://sourceforge.net/projects/freetype/files/freetype2/${FREETYPE_LEGACY_VERSION}/${FREETYPE_LEGACY_FILE}/download)
set(FREETYPE_LEGACY_HASH SHA256=5c78216d6c5860ef694fde1418d20d69d0ac83ab346c21eb311bd45709e0d93a)

# curl
set(CURL_VERSION 7.81.0)
set(CURL_PROJECT curl-${CURL_VERSION})
set(CURL_FILE ${CURL_PROJECT}.tar.bz2)
set(CURL_URL https://curl.haxx.se/download/${CURL_FILE})
set(CURL_HASH SHA256=1e7a38d7018ec060f1f16df839854f0889e94e122c4cfa5d3a37c2dc56f1e258)

# osg
set(OSG_VERSION 3.6.5)
set(OSG_PROJECT OpenSceneGraph-${OSG_VERSION})
set(OSG_FILE ${OSG_PROJECT}.zip)
set(OSG_URL https://github.com/openscenegraph/OpenSceneGraph/archive/${OSG_FILE})
set(OSG_HASH SHA256=0e9e3e4cc6f463f21a901934a95e9264b231a1d5db90f72dcb4b8cc94b0d1b3b)

# sqlite3
message(STATUS "Note the YEAR in the path AND hard-coded filename")
set(SQLITE3_VERSION 3.36.0)
set(SQLITE3_PROJECT sqlite3-${SQLITE3_VERSION})
set(SQLITE3_FILE sqlite-amalgamation-3360000.zip)
set(SQLITE3_URL https://www.sqlite.org/2021/${SQLITE3_FILE})
set(SQLITE3_HASH SHA256=999826fe4c871f18919fdb8ed7ec9dd8217180854dd1fe21eea96aed36186729)
