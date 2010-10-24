#SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ../../../cmake)
INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "usr_ls1")

SET(ROBOT_INTERFACE LEGACY WELCOME)

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp driver.cpp opponent.cpp spline.cpp pit.cpp
	strategy.cpp cardata.cpp raceline.cpp)

SET(ROBOT_FILES logo.rgb)
SET(ROBOT_SUBDIRS cars 1 2 3 4 7 8)

ROBOT(NAME ${ROBOT_NAME} INTERFACE ${ROBOT_INTERFACE}
	SOURCES ${ROBOT_SOURCES} FILES ${ROBOT_FILES}
	SUBDIRS ${ROBOT_SUBDIRS})
