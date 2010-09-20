INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "kilo2008")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp driver.cpp opponent.cpp driver.h pit.cpp strategy.cpp
		linalg.h opponent.h strategy.h pit.h cardata.h cardata.cpp spline.cpp 
		spline.h raceline.cpp raceline.h kdriver.cpp kdriver.h util.h util.cpp)

SET(ROBOT_SHIP logo.rgb)
SET(ROBOT_SHIPSUBDIRS 1 2)

ROBOT(NAME ${ROBOT_NAME} SOURCES ${ROBOT_SOURCES} SHIP ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

