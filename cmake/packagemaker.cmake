INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Speed Dreams (sdl-port) (an Open Motorsport Simulator) is a racing simulation that allows you to drive in races against opponents simulated by the computer ; it is GPL (version 2 or later) and has been forked from TORCS")
SET(CPACK_PACKAGE_NAME "speed-dreams")
SET(CPACK_PACKAGE_VENDOR "Speed Dreams Team")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "5")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_RESOURCE_FILE_LICENSE "${SOURCE_DIR}/COPYING")
SET(CPACK_RESOURCE_FILE_README "${SOURCE_DIR}/README")
SET(CPACK_PACKAGE_EXECUTABLES "speed-dreams;Speed-Dreams")

#Put Linux install information here
IF(UNIX)
	SET(CPACK_GENERATOR "DEB")
# 9.10 ubuntu depends
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "freeglut3, libalut0(>=1.1.0-1),libc6(>=2.7),libgcc1(>=1:4.1.1),libgl1-mesa-glx | libgl1,libglu1-mesa | libglu1,libice6(>=1:1.0.0),libopenal1(>=1:1.3.253),libpng12-0(>=1.2.13-4),libsm6,libstdc++6(>=4.2.1),libx11-6,libxext6,libxi6(>=2:1.1.3-1ubuntu1),libxmu6,libxrandr2,libxrender1,libxt6,libxxf86vm1,plib1.8.4c2(>=1.2.4),zlib1g(>=1:1.1.4)")
# Put other debian based distros here
ENDIF(UNIX)

#Put Windows install information here
#NSIS must be installed on your computer for this to work
IF(WIN32)
 # There is a bug in NSI that does not handle full unix paths properly. Make
 # sure there is at least one set of four (4) backlasshes.
	SET(CPACK_NSIS_MUI_ICON "${SOURCE_DIR}\\\\icon.ico")
	#SET(CPACK_PACKAGE_ICON ${CMake_SOURCE_DIR}/data/data/img\\\\splash.png)
	
ENDIF(WIN32)

SET(CPACK_PACKAGE_CONTACT "http://speed-dreams.sourceforge.net")
INCLUDE(CPack)
