INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "usr")

# Official-only USR interface.
SET(ROBOT_INTERFACE LEGACY WELCOME
                    usr_trb1
                    usr_sc
                    usr_ls1
                    )

SET(ROBOT_SOURCES
  src/usr.cpp
  src/cardata.h
  src/cardata.cpp
  src/driver.h
  src/driver.cpp
  src/linalg.h
  src/mod.h
  src/opponent.h
  src/opponent.cpp
  src/pit.h
  src/pit.cpp
  src/raceline.h
  src/raceline.cpp
  src/spline.h
  src/spline.cpp
  src/strategy.h
  src/strategy.cpp
  src/xmldefs.h)

# Official-only USR instances.
SET(ROBOT_CLONES usr_trb1 usr_sc usr_ls1)

ROBOT_MODULE(NAME ${ROBOT_NAME}
              INTERFACE ${ROBOT_INTERFACE} 
              SOURCES ${ROBOT_SOURCES}
              CLONENAMES ${ROBOT_CLONES})

# The data associated to the USR robot module and its clones.
###################
# USR core
###################
SET(ROBOT_SUBDIRS 1
                  tracks)

ROBOT_DATA(NAME ${ROBOT_NAME} PREFIX src
           #FILES README
           SUBDIRS ${ROBOT_SUBDIRS} PATTERNS *.xml logo.rgb *.png)


###################
# USR_TRB1
###################
SET(ROBOT_SUBDIRS 1 2 3 4 5 6 7 8 9 10 11 12 13 14
                  trb1-boxer-trbrs trb1-cavallo-360rb trb1-sector-rb1
                  trb1-silber-rblk trb1-spirit-rb1lt trb1-taipan-rb1 
                  trb1-zaxxon-rb
                  tracks)

ROBOT_DATA(NAME usr_trb1 PREFIX usr_trb1
           FILES usr_trb1.xml default.xml logo.rgb readme.txt
           SUBDIRS ${ROBOT_SUBDIRS} PATTERNS *.xml logo.rgb *.png)

###################
# USR_SC
###################
SET(ROBOT_SUBDIRS 1 2 3 4 5 6 7 8 9 10 11 12
                  sc-boxer-96 sc-cavallo-360 sc-fmc-gt4
                  sc-lynx-220 sc-murasama-nsx sc-spirit-300 
                  tracks)

ROBOT_DATA(NAME usr_sc PREFIX usr_sc
           FILES usr_sc.xml default.xml logo.rgb readme.txt
           SUBDIRS ${ROBOT_SUBDIRS} PATTERNS *.xml logo.rgb *.png)

###################
# USR_LS1
###################
SET(ROBOT_SUBDIRS 1 2 3 4 7 8
                  ls1-archer-r9 ls1-vulture-v6r ls1-zentek-z7r
                  tracks)

ROBOT_DATA(NAME usr_ls1 PREFIX usr_ls1
           FILES usr_ls1.xml default.xml logo.rgb readme.txt
           SUBDIRS ${ROBOT_SUBDIRS} PATTERNS *.xml logo.rgb *.png)

