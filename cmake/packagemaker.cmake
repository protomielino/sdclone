##########################################################################################
# Settings that are common to all target systems.

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   SET(CPACK_SYSTEM_NAME "win32")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   SET(CPACK_SYSTEM_NAME "macos")
ELSE()
   SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
ENDIF()

SET(INTERNAL_NAME "speed-dreams")

SET(CPACK_PACKAGE_NAME "Speed Dreams")
SET(CPACK_PACKAGE_VENDOR "The Speed Dreams team")
SET(CPACK_PACKAGE_CONTACT "https://sourceforge.net/projects/speed-dreams/")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Speed Dreams (an Open Motorsport Sim) is a racing simulation that allows you to drive in races against opponents simulated by the computer ; it is GPL 2+ and has been forked from TORCS in late 2008")
SET(CPACK_RESOURCE_FILE_LICENSE "${SOURCE_DIR}/COPYING.txt")
SET(CPACK_RESOURCE_FILE_README "${SOURCE_DIR}/README.txt")

SET(EXECUTABLE_NAME "${INTERNAL_NAME}-2")
SET(CPACK_PACKAGE_EXECUTABLES "${EXECUTABLE_NAME};Start ${CPACK_PACKAGE_NAME}")

# Version settings.
# * the short way.
#SET(CPACK_PACKAGE_VERSION "${VERSION_LONG}")

# * another way.
SET(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}${PROJECT_VERSION_METADATA}")
IF(NOT SVN_FIND_REV_FAILED)
    SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-r${SVN_REV}")
ENDIF(NOT SVN_FIND_REV_FAILED)

# Binary package settings.
SET(PACKAGE_FILE_PREFIX "${INTERNAL_NAME}")
#SET(CPACK_OUTPUT_CONFIG_FILE "/home/andy/vtk/CMake-bin/CPackConfig.cmake")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "/home/andy/vtk/CMake/Copyright.txt")

#SET(CPACK_IGNORE_FILES "/\\.svn/;\\.swp$;\\.#;/#;${CPACK_IGNORE_FILES}")

# Source package settings.
#SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/andy/vtk/CMake-bin/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE_FILE_PREFIX}-${CPACK_PACKAGE_VERSION}-src")

#SET(CPACK_RESOURCE_FILE_LICENSE "/home/andy/vtk/CMake/Copyright.txt")
#SET(CPACK_RESOURCE_FILE_README "/home/andy/vtk/CMake/Templates/CPack.GenericDescription.txt")
#SET(CPACK_RESOURCE_FILE_WELCOME "/home/andy/vtk/CMake/Templates/CPack.GenericWelcome.txt")

SET(CPACK_SOURCE_IGNORE_FILES
    "/installer/" "/doc/design/" "/doc/develdoc" "/doc/website/" "/_CPack_Packages/"
    "/CMakeCache\\\\.txt$" "/install_manifest\\\\.txt$" "/xmlversion_loc\\\\.txt$"
    "/config\\\\.h$" "/version\\\\.h$" "/doxygen_config$"
    "/\\\\.svn/" "/\\\\.dir/" "/CMakeFiles/"
    "cmake_install\\\\.cmake$" "CPackConfig\\\\.cmake$" "CPackSourceConfig\\\\.cmake$"
    "\\\\.bak$" "\\\\.flc$" "#.*#$" "~$" "\\\\.~.*"
    "\\\\.xcf$" "\\\\.xcf\\\\.bz2$" "\\\\.psd$"
    "\\\\.exe$" "/sd2-.*$" "/speed-dreams-2$" "/xmlversion$"
    "\\\\.zip$" "\\\\.tar\\\\.bz2$" "\\\\.tar\\\\.gz$" "\\\\.tar\\\\.Z$" "\\\\.tar\\\\.7z$")

##########################################################################################
# Put Linux install information here
IF(UNIX AND NOT APPLE)

    SET(PACKAGERS_BINARY "DEB" CACHE STRING "CPack binary package generators to use (separated with ';', among DEB, RPM, STGZ, TGZ, TBZ2, TZ, ZIP)")
    MARK_AS_ADVANCED(PACKAGERS_BINARY)
    SET(PACKAGERS_SOURCE "TBZ2" CACHE STRING "CPack source package generators to use (separated with ';', among TGZ, TBZ2, TZ, ZIP)")
    MARK_AS_ADVANCED(PACKAGERS_SOURCE)

    SET(CPACK_PACKAGE_NAME ${PACKAGE_FILE_PREFIX} CACHE STRING "" FORCE)

    # On debian, auto-detect dependencies
    SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    # Or manually set the dependencies
    # from Linux Mint 20 (Ubuntu 20.04)
    #SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.29), libcurl4 (>= 7.16.2), libenet7, libexpat1 (>= 2.0.1), libgcc-s1 (>= 3.0), libglu1-mesa | libglu1, libglx0, libjpeg8 (>= 8c), libopenal1 (>= 1.14), libopengl0, libopenscenegraph160, libopenthreads21, libplib1, libpng16-16 (>= 1.6.2-1), libsdl2-2.0-0 (>= 2.0.10), libsdl2-mixer-2.0-0 (>= 2.0.2), libstdc++6 (>= 9), zlib1g (>= 1:1.1.4)")

    # This causes package name to have undersocres connecting the name, version, and arch
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    # Put other Debian-based distros settings here.

    # Source package specific settings.
    LIST(APPEND CPACK_SOURCE_IGNORE_FILES "Makefile$" "\\\\.so$")

ENDIF(UNIX AND NOT APPLE)

##########################################################################################
# Put Windows install information here.
# (NSIS must be installed on your computer for this to work)
IF(WIN32)

    # General note: There is a bug in NSI that does not handle full unix paths properly.
    # Make sure there is at least one set of four (4) backlasshes.

    SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL True)

    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${INTERNAL_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}${PROJECT_VERSION_METADATA}")

    SET(EXECUTABLE_PATHNAME "$INSTDIR\\\\bin\\\\${EXECUTABLE_NAME}.exe")

    SET(PACKAGERS_BINARY "NSIS" CACHE STRING "CPack binary package generators to use (separated with ';', among NSIS, CygwinBinary, STGZ, TGZ, TBZ2, TZ, ZIP)")
    MARK_AS_ADVANCED(PACKAGERS_BINARY)
    SET(PACKAGERS_SOURCE "ZIP" CACHE STRING "CPack source package generators to use (separated with ';', among TGZ, TBZ2, TZ, ZIP)")
    MARK_AS_ADVANCED(PACKAGERS_SOURCE)

    SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_FILE_PREFIX}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-setup")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}${PROJECT_VERSION_METADATA}")
    SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}")
    #SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

    # Icon for the generated installer/uninstaller files.
    SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}\\\\data\\\\data\\\\icons\\\\icon.ico")
    SET(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}\\\\data\\\\data\\\\icons\\\\icon.ico")
    SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\data\\\\data\\\\img\\\\header.bmp")

    IF(NOT ${CMAKE_VERSION} VERSION_LESS "3.5")
       SET(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}\\\\data\\\\data\\\\img\\\\header-vert.bmp")
       SET(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}\\\\data\\\\data\\\\img\\\\header-vert.bmp")
    ENDIF(NOT ${CMAKE_VERSION} VERSION_LESS "3.5")

    IF(NOT ${CMAKE_VERSION} VERSION_LESS "3.17")
      SET(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_SOURCE_DIR}\\\\data\\\\data\\\\img\\\\header.bmp")
    ENDIF(NOT ${CMAKE_VERSION} VERSION_LESS "3.17")


    # Extra shortcuts to add in the start menu (a list of pairs : URL, Menu label).
    SET(CPACK_NSIS_MENU_LINKS
        "${CPACK_PACKAGE_CONTACT}" "Project Homepage"
        "https://sourceforge.net/p/speed-dreams/discussion/" "Community"
        "https://sourceforge.net/p/speed-dreams/tickets/" "Bug tracker"
        "/data/COPYING.txt" "License"
        "/data/README.txt" "Read me"
        "/doc/how_to_drive.html" "User manual")

    # Icon in the add/remove control panel. Must be an .exe file
    Set(CPACK_NSIS_INSTALLED_ICON_NAME "${EXECUTABLE_PATHNAME}")

    # Executable to (optionally) run after install
    SET(CPACK_NSIS_MUI_FINISHPAGE_RUN "${EXECUTABLE_NAME}")

    SET(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_PACKAGE_CONTACT}")
    SET(CPACK_NSIS_HELP_LINK "${CPACK_PACKAGE_CONTACT}")

    # Override Start menu entry
    SET(CPACK_PACKAGE_EXECUTABLES
         "${EXECUTABLE_NAME}" "${CPACK_NSIS_DISPLAY_NAME}"
         CACHE STRING "" FORCE)

    # Add a page in the install wizard for options :
    # - adding the installation path in the PATH,
    # - adding a shortcut to start the installed app on the desktop.
    #SET(CPACK_NSIS_MODIFY_PATH "ON")

    # Another way to add a shortcut to start the installed app on the desktop :
    # This ONLY works if SET(CPACK_NSIS_MODIFY_PATH "ON") which also enables the whole modify PATH page
    #SET(CPACK_CREATE_DESKTOP_LINKS "${EXECUTABLE_NAME}")

    # But this works.
    SET(SHORTCUT_TARGET "$DESKTOP\\\\${CPACK_NSIS_DISPLAY_NAME}.lnk")

    SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
         CreateShortCut \\\"${SHORTCUT_TARGET}\\\" \\\"${EXECUTABLE_PATHNAME}\\\"
        WriteRegStr HKLM 'Software\\\\${CPACK_PACKAGE_VENDOR}\\\\${CPACK_PACKAGE_INSTALL_REGISTRY_KEY}' 'VersionExtra' '${PROJECT_VERSION_METADATA}'
        WriteRegDWORD HKLM 'Software\\\\${CPACK_PACKAGE_VENDOR}\\\\${CPACK_PACKAGE_INSTALL_REGISTRY_KEY}' 'VersionMajor' '${CPACK_PACKAGE_VERSION_MAJOR}'
        WriteRegDWORD HKLM 'Software\\\\${CPACK_PACKAGE_VENDOR}\\\\${CPACK_PACKAGE_INSTALL_REGISTRY_KEY}' 'VersionMinor' '${CPACK_PACKAGE_VERSION_MINOR}'
        WriteRegDWORD HKLM 'Software\\\\${CPACK_PACKAGE_VENDOR}\\\\${CPACK_PACKAGE_INSTALL_REGISTRY_KEY}' 'VersionPatch' '${CPACK_PACKAGE_VERSION_PATCH}'
         ")

    SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \\\"${SHORTCUT_TARGET}\\\"
        ")
    IF(OPTION_TRACKEDITOR)
       set(CPACK_NSIS_CREATE_ICONS_EXTRA
          "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\SD2 TrackEditor.lnk' '$INSTDIR\\\\bin\\\\sd2-trackeditor.jar' '' '' 0 SW_SHOWNORMAL '' 'Track Editor' " )

          set(CPACK_NSIS_DELETE_ICONS_EXTRA
             "Delete '$SMPROGRAMS\\\\$START_MENU\\\\SD2 TrackEditor.lnk'" )
    ENDIF(OPTION_TRACKEDITOR)


    # Source package specific settings.
    LIST(APPEND CPACK_SOURCE_IGNORE_FILES
                "/VTune/"
                "/Release/" "/Debug/" "/RelWithDebInfo/" "/MinSizeRel/"
                "/release/" "/debug/" "/relwithdebinfo/" "/minsizerel/"
                "\\\\.sln$" "\\\\.suo$" "\\\\.ncb$" "\\\\.vcproj*$" "\\\\.dll$")

    # Add the PACKAGE_SRC project in the MSVC solution
    # (CMake 2.6 and 2.8 fail to do this itself).
    #ADD_CUSTOM_TARGET(PACKAGE_SRC)
    #ADD_CUSTOM_COMMAND(TARGET PACKAGE_SRC
    #                   COMMAND ${CMAKE_CPACK_COMMAND} -C $(OutDir) --config ./CPackSourceConfig.cmake)

ENDIF(WIN32)

##########################################################################################
# Put Mac OS X install information here
IF(APPLE)

    SET(PACKAGERS_BINARY "DragNDrop" CACHE STRING "CPack binary package generators to use (separated with ';', among Bundle, DragNDrop, PackageMaker, OSXX11, STGZ, TGZ, TBZ2, TZ, ZIP)")
    MARK_AS_ADVANCED(PACKAGERS_BINARY)
    SET(PACKAGERS_SOURCE "TBZ2" CACHE STRING "CPack source package generators to use (separated with ';', among TGZ, TBZ2, TZ, ZIP)")
    MARK_AS_ADVANCED(PACKAGERS_SOURCE)

    # Source package specific settings.
    LIST(APPEND CPACK_SOURCE_IGNORE_FILES "Makefile$")
    SET(CPACK_INSTALLED_DIRECTORIES "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX};${CMAKE_INSTALL_PREFIX}")
    SET(CPACK_INSTALL_CMAKE_PROJECTS "")
    SET(CPACK_DMG_DS_STORE "${CMAKE_SOURCE_DIR}/packaging/OSX/DS_Store-full")
    SET(CPACK_DMG_VOLUME_NAME "Speed Dreams ${VERSION}")

    # Configure the base package scripts...
     # TODO use ${CPACK_PACKAGE_NAME} ${VERSION}" ??
    SET(DMG_VOL_NAME "Speed Dreams base ${VERSION}")
    SET(DMG_FINDER_SCRIPT "findersettingsbase.scpt")

    SET(READ_WRITE_DMG_NAME "${INTERNAL_NAME}-base-${VERSION}-r${SVN_REV}-${CPACK_SYSTEM_NAME}-rw.dmg")
    SET(READ_ONLY_DMG_NAME "${INTERNAL_NAME}-base-${VERSION}-r${SVN_REV}-${CPACK_SYSTEM_NAME}.dmg")

    string(REPLACE ".app" "-base.app" SD_BASE_BUNDLE_NAME "${CMAKE_INSTALL_PREFIX}")

    # TODO make ${SD_BUNDLE_NAME} a CACHE variable and use it everywhere instead of ${CMAKE_INSTALL_PREFIX}??
    SET(SD_BUNDLE_NAME "${SD_BASE_BUNDLE_NAME}")

    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/OSX/packagedmg.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/packagebasedmg.cmake" @ONLY)

    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/OSX/findersettings.scpt.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/${DMG_FINDER_SCRIPT}" @ONLY)


    # Now configure the full package scripts...
    # TODO use ${CPACK_PACKAGE_NAME} ${VERSION}" ??
    SET(DMG_VOL_NAME "Speed Dreams ${VERSION}")
    SET(DMG_FINDER_SCRIPT "findersettingsfull.scpt")

    SET(READ_WRITE_DMG_NAME "${INTERNAL_NAME}-${VERSION}-r${SVN_REV}-${CPACK_SYSTEM_NAME}-rw.dmg")
    SET(READ_ONLY_DMG_NAME "${INTERNAL_NAME}-${VERSION}-r${SVN_REV}-${CPACK_SYSTEM_NAME}.dmg")

    # TODO make ${SD_BUNDLE_NAME} a CACHE variable and use it everywhere instead of ${CMAKE_INSTALL_PREFIX}??
    SET(SD_BUNDLE_NAME "${CMAKE_INSTALL_PREFIX}")

    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/OSX/packagedmg.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/packagefulldmg.cmake" @ONLY)

    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/OSX/findersettings.scpt.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/${DMG_FINDER_SCRIPT}" @ONLY)

   # Create a script to create the base bundle from the full bundle
   CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/packaging/OSX/createbaseapp.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/createbaseapp.cmake" @ONLY)

ENDIF(APPLE)

##########################################################################################
# Final settings.
SET(CPACK_GENERATOR ${PACKAGERS_BINARY})
SET(CPACK_SOURCE_GENERATOR ${PACKAGERS_SOURCE})

#INCLUDE(CPack)
