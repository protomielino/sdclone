INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "hymie")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp cardata.cpp  correct.cpp  driver.cpp
	learn.cpp  opponent.cpp  pit.cpp  spline.cpp  strategy.cpp cardata.h
	correct.h  driver.h  learn.h  linalg.h  name.h  opponent.h  pit.h
	spline.h  strategy.h)

SET(ROBOT_SHIP)
SET(ROBOT_SHIPSUBDIRS 1 2 3 4 5 6 7 8 9 10 sc-996 sc-ciclon-300 sc-f360 sc-gt4 sc-lynx-220 sc-nsx svg)

ROBOT(NAME ${ROBOT_NAME} SOURCES ${ROBOT_SOURCES} SHIP ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

