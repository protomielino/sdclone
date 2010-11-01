#SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ../../../cmake)
INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "usr_ls1")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp driver.cpp opponent.cpp spline.cpp pit.cpp
	strategy.cpp cardata.cpp raceline.cpp)

SET(ROBOT_SUBDIRS cars 1 2 3 4 7 8)

ROBOT_MODULE(NAME ${ROBOT_NAME}
			 INTERFACE LEGACY WELCOME
             SOURCES ${ROBOT_SOURCES})

ROBOT_DATA(NAME ${ROBOT_NAME}
           FILES ${ROBOT_NAME}.xml logo.rgb
           SUBDIRS ${ROBOT_SUBDIRS} PATTERNS *.xml logo.rgb *.png)
