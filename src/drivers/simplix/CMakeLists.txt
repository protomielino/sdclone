INCLUDE(../../../cmake/macros.cmake)


# The ubiquitous robot module and its clones.
SET(ROBOT_NAME "simplix")

SET(ROBOT_INTERFACE LEGACY WELCOME
		    simplix_mp5 simplix_mpa1 simplix_trb1a simplix_trb1b simplix_sca simplix_scb simplix_36GP simplix_ls1 
		     my_simplix_0 my_simplix_1 my_simplix_2 my_simplix_3 my_simplix_4
		    my_simplix_5 my_simplix_6 my_simplix_7 my_simplix_8 my_simplix_9)

SET(ROBOT_SOURCES 
	src/unitcarparam.cpp
	src/unitcarparam.h
	src/unitcharacteristic.cpp
	src/unitcharacteristic.h
	src/unitclothoid.cpp
	src/unitclothoid.h
	src/unitcollision.cpp
	src/unitcollision.h
	src/unitcommon.cpp
	src/unitcommondata.cpp
	src/unitcommondata.h
	src/unitcommon.h
	src/unitcubic.cpp
	src/unitcubic.h
	src/unitcubicspline.cpp
	src/unitcubicspline.h
	src/unitdriver.cpp
	src/unitdriver.h
	src/unitfixcarparam.cpp
	src/unitfixcarparam.h
	src/unitglobal.h
	src/unitlane.cpp
	src/unitlane.h
	src/unitlanepoint.cpp
	src/unitlanepoint.h
	src/unitlinalg.cpp
	src/unitlinalg.h
	src/unitlinreg.cpp
	src/unitlinreg.h
	src/unitmain.cpp
	src/unitopponent.cpp
	src/unitopponent.h
	src/unitparabel.cpp
	src/unitparabel.h
	src/unitparam.cpp
	src/unitparam.h
	src/unitpidctrl.cpp
	src/unitpidctrl.h
	src/unitpit.cpp
	src/unitpit.h
	src/unitpitparam.cpp
	src/unitpitparam.h
	src/unitsection.cpp
	src/unitsection.h
	src/unitstrategy.cpp
	src/unitstrategy.h
	src/unitsysfoo.cpp
	src/unitsysfoo.h
	src/unitteammanager.cpp
	src/unitteammanager.h
	src/unittmpcarparam.cpp
	src/unittmpcarparam.h
	src/unittrack.cpp
	src/unittrack.h
	src/unitvec2d.cpp
	src/unitvec2d.h
	src/unitvec3d.cpp
	src/unitvec3d.h
)

# Official-only simplix instances.
IF(OPTION_OFFICIAL_ONLY)

	SET(ROBOT_CLONES simplix_36GP simplix_sc simplix_trb1 simplix_ls1 simplix_mp5)

# Official and work-in-progress / unofficial simplix instances.
ELSE()

	SET(ROBOT_CLONES simplix_36GP simplix_sc simplix_trb1 simplix_ls1 simplix_mp5
					 simplix_mpa1 )

ENDIF()

ROBOT_MODULE(NAME ${ROBOT_NAME} INTERFACE ${ROBOT_INTERFACE} SOURCES ${ROBOT_SOURCES}
				  CLONENAMES ${ROBOT_CLONES})

# The data associated to the ubiquitous robot module and its clones.
SET(ROBOT_FILES logo.rgb readme.txt default.xml)

SET(ROBOT_SUBDIRS 0 trb1-cavallo-360rb tracks
					  ls1-archer-r9 ls1-cavallo-570s1 ls1-newcastle-fury ls1-taipan-ltsr
					  ls1-toro-rgt ls1-vulture-v6r ls1-zentek-z7r
                      sc-boxer-96 sc-cavallo-360 sc-fmc-gt4 sc-lynx-220 
					  sc-murasama-nsx sc-spirit-300)

ROBOT_DATA(NAME ${ROBOT_NAME} PREFIX src 
           FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})

# 36GP
SET(ROBOT_SUBDIRS 0 1 2 3 4 5 6 7 8 9
					  36gp-ettore-t59 36gp-milano-12c36 36gp-silber-w25b 36gp-tridenti-6c34 36gp-tridenti-v8ri
                      tracks)

ROBOT_DATA(NAME simplix_36GP PREFIX simplix_36GP
           FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})

# SuperCars
SET(ROBOT_SUBDIRS 0 1 2 3 4 5 6 7 8 12 13
                      sc-boxer-96 sc-spirit-300 sc-cavallo-360 sc-fmc-gt4 sc-lynx-220 sc-murasama-nsx
                      tracks)

ROBOT_DATA(NAME simplix_sc PREFIX simplix_sc
           FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})
           
# LS-GT1
SET(ROBOT_SUBDIRS 0 1 2 3 4 5 8 9 12 13
                      ls1-toro-rgt ls1-cavallo-570s1 ls1-newcastle-fury ls1-archer-r9
                      ls1-zentek-z7r ls1-taipan-ltsr ls1-vulture-v6r 
                      tracks)

ROBOT_DATA(NAME simplix_ls1 PREFIX simplix_ls1
           FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})

# TRB1
SET(ROBOT_SUBDIRS 0 1 2 3 4 5 6 7 8 9 10 11 12 13
                      trb1-boxer-trbrs trb1-sector-rb1 trb1-spirit-rb1lt
					  trb1-cavallo-360rb trb1-silber-rblk trb1-taipan-rb1 trb1-zaxxon-rb
                      tracks)

ROBOT_DATA(NAME simplix_trb1 PREFIX simplix_trb1
           FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})

# MP5
SET(ROBOT_SUBDIRS 0 1 2 3 4 5 mp5-fmc tracks)

ROBOT_DATA(NAME simplix_mp5 PREFIX simplix_mp5
           FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})


# Work-in-progress / unofficial simplix instances.
IF(NOT OPTION_OFFICIAL_ONLY)

	SET(ROBOT_SUBDIRS 0 1 2 3 4 5 6 7 8 9 mpa1-murasama tracks)

	ROBOT_DATA(NAME simplix_mpa1 PREFIX simplix_mpa1
			   FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})

ENDIF()