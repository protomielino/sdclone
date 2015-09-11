INCLUDE(../../cmake/macros.cmake)

PROJECT(speed-dreams-2)

SET(_TARGET_NAME speed-dreams-2)

SET(_SOURCES main.cpp)
IF(MSVC)
	SET(_SOURCES ${_SOURCES} main.rc resource.h ../../data/data/icons/icon.ico)
ENDIF(MSVC)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(tgf tgfclient tgfdata math portability)

IF(OPTION_SDL2)
	ADD_SDL2_INCLUDEDIR()
ELSE(OPTION_SDL2)
	ADD_SDL_INCLUDEDIR()
ENDIF(OPTION_SDL2)

ADD_PLIB_INCLUDEDIR()

# Disable developer warning
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(MSVC)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG 
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcmt.lib")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE 
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmt.lib")
ENDIF(MSVC)

SD_ADD_EXECUTABLE(${_TARGET_NAME} ${_SOURCES})

ADD_SDLIB_LIBRARY(${_TARGET_NAME} portability tgf tgfclient tgfdata)

# Needed when using the "MinGW Makefiles" CMake generator (but not with the "MSYS Makefiles" one !)
IF(MINGW)
	TARGET_LINK_LIBRARIES(${_TARGET_NAME} ${MINGW32_LIBRARY})
ENDIF(MINGW)

IF(UNIX)
	SD_INSTALL_FILES(MAN man6 PREFIX ${SOURCE_DIR}/doc/man FILES ${_TARGET_NAME}.6)
ENDIF(UNIX)

SD_INSTALL_FILES(BIN TARGETS ${_TARGET_NAME})

IF(APPLE)
   INSTALL(FILES Info.plist DESTINATION ./Contents)
   SET(APPS "\${CMAKE_INSTALL_PREFIX}/games/${_TARGET_NAME}")
   SET(DIRS /opt/local)
   SET(DIRS ${DIRS} "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/lib")

   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/graphic/ssggraph.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/racing/standardgame.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/simu/simuv4.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/sound/snddefault.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/graphic/ssggraph.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/track/trackv1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/modules/userinterface/legacymenu.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/dandroid/dandroid.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/dandroid_36GP/dandroid_36GP.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/dandroid_mpa12/dandroid_mpa12.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/human/human.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/kilo2008/kilo2008.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/networkhuman/networkhuman.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/replay/replay.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow/shadow.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_36GP/shadow_36GP.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_lp1/shadow_lp1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_ls1/shadow_ls1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_ls2/shadow_ls2.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_mp5/shadow_mp5.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_mpa1/shadow_mpa1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_mpa11/shadow_mpa11.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_mpa12/shadow_mpa12.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_ref/shadow_ref.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_sc/shadow_sc.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_srw/shadow_srw.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/shadow_trb1/shadow_trb1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix/simplix.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_36GP/simplix_36GP.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_lp1/simplix_lp1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_ls1/simplix_ls1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_ls2/simplix_ls2.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_mp5/simplix_mp5.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_mpa1/simplix_mpa1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_mpa11/simplix_mpa11.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_mpa12/simplix_mpa12.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_ref/simplix_ref.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_sc/simplix_sc.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_srw/simplix_srw.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/simplix_trb1/simplix_trb1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr/usr.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_36GP/usr_36GP.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_lp1/usr_lp1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_ls1/usr_ls1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_ls2/usr_ls2.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_mpa1/usr_mpa1.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_mpa11/usr_mpa11.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_mpa12/usr_mpa12.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_rs/usr_rs.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_sc/usr_sc.so")
   LIST(APPEND PlugIns "\${CMAKE_INSTALL_PREFIX}/lib64/games/${_TARGET_NAME}/drivers/usr_trb1/usr_trb1.so")


# These are installed by their respective CMakeLists.txt
# Executables
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-accc)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-menuview)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-nfs2ac)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-nfsperf)
#LIST(APPEND PlugIns ${CMAKE_INSTALL_PREFIX}/games/sd2-sd2-trackgen)

   INSTALL(CODE "
   INCLUDE(BundleUtilities)
   SET(BU_CHMOD_BUNDLE_ITEMS ON)
   FIXUP_BUNDLE(\"${APPS}\" \"${PlugIns}\" \"${DIRS}\")")

ENDIF(APPLE)

# Under Windows, install needed 3rd party DLLs close to Speed Dreams executable.
IF(WIN32)
	INCLUDE(customthirdparty)
	SD_INSTALL_CUSTOM_3RDPARTY(${_TARGET_NAME})
ENDIF(WIN32)
