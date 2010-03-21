INCLUDE(../../../cmake/macros.cmake)


# The ubiquitous robot module and its clones.
SET(ROBOT_NAME "simplix")

SET(ROBOT_INTERFACE LEGACY WELCOME
		    simplix_indy simplix_trb1a simplix_trb1b simplix_sca simplix_scb simplix_36GP simplix_ls1
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

SET(ROBOT_CLONES simplix_indy simplix_36GP simplix_sc simplix_trb1 simplix_ls1)

ROBOT_MODULE(NAME ${ROBOT_NAME} INTERFACE ${ROBOT_INTERFACE} SOURCES ${ROBOT_SOURCES}
	     CLONENAMES ${ROBOT_CLONES})

# The data associated to the ubiquitous robot module and its clones.
SET(ROBOT_SHIP logo.rgb readme.txt)

SET(ROBOT_SHIPSUBDIRS 0 trb1-cavallo-360rb tracks)
ROBOT_DATA(NAME ${ROBOT_NAME} SHIPPREFIX src 
           SHIPFILES ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

SET(ROBOT_SHIPSUBDIRS 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
		      25 26 27 28 29
                      indycar01 indycar02 indycar03 indycar04 indycar05 indycar06 indycar07
					  indycar08 indycar09 indycar10 indycar11 indycar12 indycar13 indycar14
                      indycar15 tracks)

ROBOT_DATA(NAME simplix_indy SHIPPREFIX simplix_indy
           SHIPFILES ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

SET(ROBOT_SHIPSUBDIRS 0 1 2 3 4 5 6 7 8 9
                      36GP-alfa12c 36GP-bugatti 36GP-maserati 36GP-maserati6C34 36GP-mercedes
                      tracks)

ROBOT_DATA(NAME simplix_36GP SHIPPREFIX simplix_36GP
           SHIPFILES ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

SET(ROBOT_SHIPSUBDIRS 0 1 2 3 4 5 6 7 8 12 13
                      sc-boxer-96 sc-spirit-300 sc-cavallo-360 sc-fmc-gt4 sc-lynx-220 sc-murasama-nsx
                      tracks)

ROBOT_DATA(NAME simplix_sc SHIPPREFIX simplix_sc
           SHIPFILES ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})
           
SET(ROBOT_SHIPSUBDIRS 0 1 2 3 4 5 8 9 12 13
                      ls1-toro-rgt ls1-cavallo-570s1 ls1-newcastle-fury ls1-archer-r9
                      ls1-zentek-z7r ls1-taipan-ltsr ls1-vulture-v6r 
                      tracks)

ROBOT_DATA(NAME simplix_ls1 SHIPPREFIX simplix_ls1
           SHIPFILES ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

SET(ROBOT_SHIPSUBDIRS 0 1 2 3 4 5 6 7 8 9 10 11 12 13
                      trb1-boxer-trbrs trb1-sector-rb1 trb1-spirit-rb1lt
		      trb1-cavallo-360rb trb1-silber-rblk trb1-taipan-rb1 trb1-zaxxon-rb
                      tracks)

ROBOT_DATA(NAME simplix_trb1 SHIPPREFIX simplix_trb1
           SHIPFILES ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

