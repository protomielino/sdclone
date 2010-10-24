INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "kilo2008")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp opponent.cpp pit.cpp strategy.cpp
    linalg.h opponent.h strategy.h pit.h cardata.h cardata.cpp spline.cpp 
    spline.h raceline.cpp raceline.h kdriver.cpp kdriver.h util.h util.cpp)

SET(ROBOT_FILES logo.rgb default.xml)
SET(ROBOT_SUBDIRS 1 2 5 6 tracks trb1-cavallo-360rb ls1-archer-r9)

ROBOT(NAME ${ROBOT_NAME} SOURCES ${ROBOT_SOURCES} FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})

