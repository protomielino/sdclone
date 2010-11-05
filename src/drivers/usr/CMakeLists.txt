INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "usr")

# Official-only USR interface.
SET(ROBOT_INTERFACE LEGACY WELCOME
                    usr_trb1
                    )

# Work-in-progress / unofficial simplix interface.
IF(NOT OPTION_OFFICIAL_ONLY)
    SET(ROBOT_INTERFACE ${ROBOT_INTERFACE} usr_mpa1)
ENDIF()

SET(ROBOT_SOURCES
  ${ROBOT_NAME}.cpp
  cardata.h
  cardata.cpp
  driver.h
  driver.cpp
  linalg.h
  mod.h
  opponent.h
  opponent.cpp
  pit.h
  pit.cpp
  raceline.h
  raceline.cpp
  spline.h
  spline.cpp
  strategy.h
  strategy.cpp
  xmldefs.h)

# Official-only USR instances.
SET(ROBOT_CLONES usr_trb1)

IF(NOT OPTION_OFFICIAL_ONLY)

  # Work-in-progress / unofficial USR instances.
    SET(ROBOT_CLONES ${ROBOT_CLONES} usr_mpa1)

ENDIF()

ROBOT_MODULE(NAME ${ROBOT_NAME}
              INTERFACE ${ROBOT_INTERFACE} 
              SOURCES ${ROBOT_SOURCES}
              CLONENAMES ${ROBOT_CLONES})

# The data associated to the USR robot module and its clones.
# USR_TRB1
SET(ROBOT_SUBDIRS 1 2 3 4 5 6 7 8 9 10 11 12 13 14
                  trb1-boxer-trbrs trb1-cavallo-360rb trb1-sector-rb1
                  trb1-silber-rblk trb1-spirit-rb1lt trb1-taipan-rb1 
                  trb1-zaxxon-rb
                  tracks)

ROBOT_DATA(NAME usr_trb1 PREFIX usr_trb1 
           FILES usr_trb1.xml default.xml logo.rgb readme.txt
           SUBDIRS ${ROBOT_SUBDIRS} PATTERNS *.xml logo.rgb *.png)

