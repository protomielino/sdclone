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


#SDL
set(SDL1_VERSION 1.2.15)
set(SDL1_PROJECT SDL-${SDL1_VERSION})
set(SDL1_FILE ${SDL1_PROJECT}.tar.gz)
set(SDL1_URL https://www.libsdl.org/release/${SDL1_FILE})
set(SDL1_HASH SHA256=d6d316a793e5e348155f0dd93b979798933fb98aa1edebcc108829d6474aad00)

# SDL2
set(SDL2_VERSION 2.0.14)
set(SDL2_PROJECT SDL2-${SDL2_VERSION})
set(SDL2_FILE ${SDL2_PROJECT}.tar.gz)
set(SDL2_URL https://www.libsdl.org/release/${SDL2_FILE})
set(SDL2_HASH SHA256=d8215b571a581be1332d2106f8036fcb03d12a70bae01e20f424976d275432bc)

# OpenAL-soft
set(OPENAL_PROJECT_VERSION 1.21.1)
set(OPENAL_PROJECT openal-soft-${OPENAL_PROJECT_VERSION})
set(OPENAL_FILE ${OPENAL_PROJECT}.tar.bz2)
set(OPENAL_URL https://www.openal-soft.org/openal-releases/${OPENAL_FILE})
set(OPENAL_HASH SHA256=c8ad767e9a3230df66756a21cc8ebf218a9d47288f2514014832204e666af5d8)

message(WARNING "openal-soft versions newer than 1.19.1 need Visual Studio 2017 and CMAKE_SYSTEM_VERSION >= 10")
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
set(PLIB_HEAD_HASH SHA256=f82b5bb97f975879d55785ba373d16b0e3d6ef0507dd3fd008a2aa78e0bd4aad)

set(PLIB_VERSION 1.8.5)
set(PLIB_PROJECT plib-${PLIB_VERSION})
set(PLIB_FILE ${PLIB_PROJECT}.tar.gz)
set(PLIB_URL http://plib.sourceforge.net/dist/${PLIB_FILE})
set(PLIB_HASH SHA256=485b22bf6fdc0da067e34ead5e26f002b76326f6371e2ae006415dea6a380a32)

# jpeg
set(JPEG_VERSION 9d)
set(JPEG_PROJECT jpeg-${JPEG_VERSION})
set(JPEG_FILE jpegsrc.v${JPEG_VERSION}.tar.gz)
set(JPEG_URL https://ijg.org/files/${JPEG_FILE})
set(JPEG_HASH SHA256=6c434a3be59f8f62425b2e3c077e785c9ce30ee5874ea1c270e843f273ba71ee)

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
set(OGG_VERSION 1.3.4)
set(OGG_PROJECT ogg-${OGG_VERSION})
set(OGG_FILE lib${OGG_PROJECT}.tar.gz)
set(OGG_URL https://downloads.xiph.org/releases/ogg/${OGG_FILE})
set(OGG_HASH SHA256=fe5670640bd49e828d64d2879c31cb4dde9758681bb664f9bdbf159a01b0c76e)

# vorbis
set(VORBIS_VERSION 1.3.7)
set(VORBIS_PROJECT vorbis-${VORBIS_VERSION})
set(VORBIS_FILE lib${VORBIS_PROJECT}.tar.gz)
set(VORBIS_URL https://downloads.xiph.org/releases/vorbis/${VORBIS_FILE})
set(VORBIS_HASH SHA256=0e982409a9c3fc82ee06e08205b1355e5c6aa4c36bca58146ef399621b0ce5ab)

# expat
message(STATUS "Note special path handling (version in path)")
set(EXPAT_VERSION 2.2.10)
set(EXPAT_PROJECT expat-${EXPAT_VERSION})
set(EXPAT_FILE ${EXPAT_PROJECT}.tar.bz2)
set(EXPAT_URL https://sourceforge.net/projects/expat/files/expat/${EXPAT_VERSION}/${EXPAT_FILE}/download)
set(EXPAT_HASH SHA256=b2c160f1b60e92da69de8e12333096aeb0c3bf692d41c60794de278af72135a5)

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
set(FREETYPE_VERSION 2.10.4)
set(FREETYPE_PROJECT freetype-${FREETYPE_VERSION})
set(FREETYPE_FILE ft2104.zip)
set(FREETYPE_URL https://sourceforge.net/projects/freetype/files/freetype2/${FREETYPE_VERSION}/${FREETYPE_FILE}/download)
set(FREETYPE_HASH SHA256=5c78216d6c5860ef694fde1418d20d69d0ac83ab346c21eb311bd45709e0d93a)

# curl
set(CURL_VERSION 7.75.0)
set(CURL_PROJECT curl-${CURL_VERSION})
set(CURL_FILE ${CURL_PROJECT}.tar.bz2)
set(CURL_URL https://curl.haxx.se/download/${CURL_FILE})
set(CURL_HASH SHA256=50552d4501c178e4cc68baaecc487f466a3d6d19bbf4e50a01869effb316d026)

# osg
set(OSG_VERSION 3.6.5)
set(OSG_PROJECT OpenSceneGraph-${OSG_VERSION})
set(OSG_FILE ${OSG_PROJECT}.zip)
set(OSG_URL https://github.com/openscenegraph/OpenSceneGraph/archive/${OSG_FILE})
set(OSG_HASH SHA256=0e9e3e4cc6f463f21a901934a95e9264b231a1d5db90f72dcb4b8cc94b0d1b3b)

# sqlite3
message(STATUS "Note the YEAR in the path AND hard-coded filename")
set(SQLITE3_VERSION 3.33.0)
set(SQLITE3_PROJECT sqlite3-${SQLITE3_VERSION})
set(SQLITE3_FILE sqlite-amalgamation-3330000.zip)
set(SQLITE3_URL https://www.sqlite.org/2020/${SQLITE3_FILE})
set(SQLITE3_HASH SHA256=b34f4c0c0eefad9a7e515c030c18702e477f4ef7d8ade6142bdab8011b487ac6)
