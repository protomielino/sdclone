INCLUDE(../../../cmake/macros.cmake)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

SET(ROBOT_NAME "usr")

# Official-only USR interface.
SET(ROBOT_INTERFACE LEGACY WELCOME)

SET(ROBOT_SOURCES
  src/usr.cpp
  src/cardata.h
  src/cardata.cpp
  src/driver.h
  src/driver.cpp
  src/learning.h
  src/linalg.h
  src/line.cpp
  src/line.h
  src/linemode.h
  src/linemode.cpp
  src/manual_override.cpp
  src/manual_override.h
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
  #src/tiresdata.cpp
  #src/tiresdata.h
  src/vec2d.h
  src/globaldefs.h)

# Official-only USR instances.
SET(ROBOT_CLONES usr_trb1 usr_sc usr_srw usr_ls1 usr_ls2 usr_mpa1 usr_mpa11 usr_mpa12 usr_36GP usr_rs usr_lp1 usr_mp10)

ROBOT_MODULE(NAME ${ROBOT_NAME} VERSION 3.0.0 SOVERSION 1.0.0
              INTERFACE ${ROBOT_INTERFACE}
              SOURCES ${ROBOT_SOURCES}
              CLONENAMES ${ROBOT_CLONES})

# For data associated to the ubiquirous USR robot module and its clones,
# see data/drivers/usr*/CMakeLists.txt
