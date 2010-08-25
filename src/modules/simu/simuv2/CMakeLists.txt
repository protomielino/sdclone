INCLUDE(../../../../cmake/macros.cmake)

ADD_TORCSLIB_INCLUDEDIR(portability)

INCLUDE_DIRECTORIES(../simuv2.1/SOLID-2.0/include)

SET(SIMUV2_SOURCES aero.cpp axle.cpp brake.cpp car.cpp categories.cpp
                   collide.cpp differential.cpp engine.cpp simu.cpp simuitf.cpp
                   steer.cpp susp.cpp transmission.cpp wheel.cpp
                   aero.h axle.h brake.h carstruct.h differential.h engine.h
                   sim.h steer.h susp.h transmission.h wheel.h)


ADD_INTERFACE_INCLUDEDIR()
ADD_OSSPEC_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(math tgf robottools)
INCLUDE_DIRECTORIES(SOLID-2.0/include)

IF(WIN32)
	ADD_LIBRARY(simuv2 SHARED ${SIMUV2_SOURCES} simuv2.def)
ELSE(WIN32)
	ADD_LIBRARY(simuv2 SHARED ${SIMUV2_SOURCES})
ENDIF(WIN32)

IF(UNIX)
	SET_TARGET_PROPERTIES(simuv2 PROPERTIES PREFIX "")
ENDIF(UNIX)

ADD_TORCSLIB_LIBRARY(simuv2 tgf robottools solid)

TORCS_INSTALL_FILES(LIB modules/simu TARGETS simuv2)

