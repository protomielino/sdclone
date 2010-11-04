#SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ../../../cmake)
INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "usr")

SET(ROBOT_SOURCES
	${ROBOT_NAME}.cpp
	cardata.cpp
	driver.cpp
	opponent.cpp
	pit.cpp
	raceline.cpp
	spline.cpp
	strategy.cpp)

ROBOT_MODULE(NAME ${ROBOT_NAME}
			 INTERFACE LEGACY WELCOME
             SOURCES ${ROBOT_SOURCES})

ROBOT_DATA(NAME ${ROBOT_NAME}
           FILES ${ROBOT_NAME}.xml logo.rgb
           PATTERNS *.xml logo.rgb *.png)
