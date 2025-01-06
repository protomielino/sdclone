#CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12 FATAL_ERROR)
CMAKE_MINIMUM_REQUIRED(VERSION 3.5 FATAL_ERROR)

# Speed Dreams project
PROJECT(speed-dreams-2 VERSION 2.3.0)

MESSAGE(STATUS CMAKE_VERSION = "${CMAKE_VERSION}")

IF(POLICY CMP0003)
   CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(POLICY CMP0003)

IF(POLICY CMP0048)
   CMAKE_POLICY(SET CMP0048 NEW)
ENDIF(POLICY CMP0048)

IF(POLICY CMP0056)
   CMAKE_POLICY(SET CMP0056 NEW)
ENDIF(POLICY CMP0056)

IF(POLICY CMP0066)
  CMAKE_POLICY(SET CMP0066 NEW)
ENDIF(POLICY CMP0066)

IF(POLICY CMP0072)
  CMAKE_POLICY(SET CMP0072 NEW)
ENDIF(POLICY CMP0072)

IF(APPLE)
   SET(CMAKE_CXX_STANDARD 11)
   SET(CMAKE_CXX_STANDARD_REQUIRED True)
ENDIF(APPLE)

option(SD_ASSUME_DATADIR "Assume default directory for speed-dreams-data if not found")

set(data_version 2.3.0)
find_package(speed-dreams-data ${data_version})

if(NOT SD_DATADIR)
   if(SD_ASSUME_DATADIR)
      include(GNUInstallDirs)

      set(SD_DATADIR ${CMAKE_INSTALL_DATADIR}/games/speed-dreams-2)
      set(SD_DATADIR_ABS ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/games/speed-dreams-2)
      message(STATUS "Data directory: ${SD_DATADIR_ABS} (assumed)")
      set(SD_HAS_DATADIR true)
   else()
      message(WARNING "Data directory not found or incompatible with version "
         "${data_version}. Speed Dreams should still "
         "be able to build, but it might not run successfully. "
         "If speed-dreams-data has been installed to a non-standard location, "
         "please add -DCMAKE_PREFIX_PATH=<data-dir> to the command line to "
         "find it.")
   endif()
else()
   message(STATUS "Data directory: ${SD_DATADIR_ABS}")
   set(SD_HAS_DATADIR true)
endif()

INCLUDE(cmake/macros.cmake)
INCLUDE(cmake/checks.cmake)

CHECK_HEADERS_H()
CHECK_FUNCTIONS()
CHECK_LIBRARIES()

execute_process(
   COMMAND git describe --tags --dirty
   RESULT_VARIABLE result
   OUTPUT_VARIABLE VERSION
   OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT result EQUAL 0)
   set(VERSION "undefined-version")
endif()

set(VERSION_LONG ${VERSION})

# PROJECT_VERSION_METADATA is used by NSIS packagers
# its what comes after the '-' sign (if anything)
# ie: in tag '2.2.2-rc1' PROJECT_VERSION_METADATA should be '-rc1'
STRING(FIND "${VERSION}" "-" _METADATA_INDEX)
IF(NOT ${_METADATA_INDEX} EQUAL -1)
   STRING(SUBSTRING ${VERSION} ${_METADATA_INDEX} -1 PROJECT_VERSION_METADATA)
ELSE(NOT ${_METADATA_INDEX} EQUAL -1)
   SET(PROJECT_VERSION_METADATA )
ENDIF(NOT ${_METADATA_INDEX} EQUAL -1)
#MESSAGE(STATUS "PROJECT_VERSION_METADATA ${PROJECT_VERSION_METADATA}")

# Generate config.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Generate DOxygen configuration file
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/doxygenconfig.in.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen_config @ONLY)

# Setup empty GLOBAL lists for robots and modules
SET_PROPERTY(GLOBAL PROPERTY SD_MODULE_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_CAR_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_TRACK_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_ROBOT_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_OSG_PLUGIN_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SD_EXE_LIST "")

# Sub-dirs to build ...
ADD_SUBDIRECTORY(src)

IF(MSVC)
   INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/faq/faq.html DESTINATION ./doc)
   INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/userman/how_to_drive.html DESTINATION ./doc)
   INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/userman/images/ DESTINATION ./doc/images/)
ENDIF(MSVC)


# Useful for using an installation of SD as a robot development framework without building it.
# Only do for an in source tree build as INSTALL_SD_CMAKE() doesn't seem to use CMAKE_INSTALL_PREFIX
IF("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
   INSTALL_SD_CMAKE()
ENDIF("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")

# Must be the last ADD_SUBDIRECTORY for FIXUP_BUNDLE to work
ADD_SUBDIRECTORY(packaging)

# Generate a 'make clobber'-like clobber.sh/.bat script in case of an in-source build.
IF("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	SD_GENERATE_CLOBBER_SCRIPT()
ENDIF("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")

# Add an unistall target.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake" IMMEDIATE @ONLY)
IF(MSVC)
	SET(_UNINST_TGT_NAME "UNINSTALL")
ELSE(MSVC)
	SET(_UNINST_TGT_NAME "uninstall")
ENDIF(MSVC)
ADD_CUSTOM_TARGET(${_UNINST_TGT_NAME} "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake" )

IF(OPTION_CHECK_CONTENTS)
   INCLUDE(cmake/prerelease.cmake)
ENDIF(OPTION_CHECK_CONTENTS)

IF(OPTION_PACKAGING OR OPTION_ALL_IN_ONE_PACKAGING)
   INCLUDE(cmake/packagemaker.cmake)
ENDIF(OPTION_PACKAGING OR OPTION_ALL_IN_ONE_PACKAGING)
IF(OPTION_PACKAGING)
   IF(MSVC)
      INCLUDE(cmake/packagemaker-nsis.cmake)
   ENDIF(MSVC)
   IF(APPLE)
      INCLUDE(cmake/packagemaker-dmg.cmake)
   ENDIF(APPLE)
ENDIF(OPTION_PACKAGING)

IF(OPTION_ALL_IN_ONE_PACKAGING)
   IF(MSVC AND ${CMAKE_VERSION} VERSION_LESS "3.17")
      MESSAGE(SEND_ERROR "OPTION_ALL_IN_ONE_PACKAGING requires CMake 3.17 or greater on Windows")
   ENDIF(MSVC AND ${CMAKE_VERSION} VERSION_LESS "3.17")
   INCLUDE(CPack)
ENDIF(OPTION_ALL_IN_ONE_PACKAGING)

IF(OPTION_PORTABLE_PACKAGING)
   IF(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
         #[[
         This section adds a batch file for running speed-dreams-2.exe with a local settings directory.
         This is similar to how 'Portable Apps' work.
         It also adds projects to build a self-extracting 7-zip file (if 7-zip is installed)
      ]]
   ENDIF(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
   IF(WIN32)

      find_program(7ZIP_EXE 7z.exe PATH_SUFFIXES 7-Zip)
      find_file(7ZIP_WIN 7z.sfx PATH_SUFFIXES 7-Zip)

      set(SD_EXE_NAME "speed-dreams-2.exe")
      SET(PORTABLE_PACKAGE_FILE_NAME "speed-dreams-2-${VERSION_LONG}-portable-setup" CACHE STRING "Name for portable package file")
      SET(PORTABLE_BATCH_FILE_NAME "speed-dreams-2-${VERSION_LONG}-portable.bat" CACHE STRING "Name for portable batch file")

            CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/portable/sd-portable.bat.in"
                  "${CMAKE_BINARY_DIR}/${PORTABLE_BATCH_FILE_NAME}" @ONLY)

      INSTALL(PROGRAMS "${CMAKE_BINARY_DIR}/${PORTABLE_BATCH_FILE_NAME}" DESTINATION .)

      # NOTE this WILL pick up changes and additions!
      # just re-build INSTALL no need to rerun CMake configure!
      INSTALL(DIRECTORY packaging/portable/settings DESTINATION . )

      if(7ZIP_EXE AND 7ZIP_WIN)

         ADD_CUSTOM_TARGET(PORTABLE_INSTALL)
         ADD_CUSTOM_TARGET(PORTABLE_PACKAGE DEPENDS PORTABLE_INSTALL)

         ADD_CUSTOM_COMMAND(TARGET PORTABLE_INSTALL
                     COMMAND "${CMAKE_COMMAND}" --build . --target INSTALL --config $<CONFIG>
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                     COMMENT "Building INSTALL...")

         ADD_CUSTOM_COMMAND(TARGET PORTABLE_PACKAGE
                        COMMAND ${7ZIP_EXE} a -sfx7z.sfx ${CMAKE_BINARY_DIR}/${PORTABLE_PACKAGE_FILE_NAME} *
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}
                        COMMENT "Building portable package...")

      else(7ZIP_EXE AND 7ZIP_WIN)
            message(STATUS "Could not find 7-ZIP. Portable build disabled.")
      endif(7ZIP_EXE AND 7ZIP_WIN)
   ENDIF(WIN32)
ENDIF(OPTION_PORTABLE_PACKAGING)
