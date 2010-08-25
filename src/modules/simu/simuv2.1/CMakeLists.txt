ADD_SUBDIRECTORY(SOLID-2.0)

INCLUDE(../../../../cmake/macros.cmake)

ADD_TORCSLIB_INCLUDEDIR(portability)

#SOLID-source: it doesn't do the subdirectory first
SET(SOLID_SOURCES SOLID-2.0/src/Transform.cpp SOLID-2.0/src/Convex.cpp
                  SOLID-2.0/src/Box.cpp SOLID-2.0/src/Cone.cpp SOLID-2.0/src/Cylinder.cpp
                  SOLID-2.0/src/Sphere.cpp SOLID-2.0/src/Simplex.cpp
		  SOLID-2.0/src/Polygon.cpp SOLID-2.0/src/Polyhedron.cpp
		  SOLID-2.0/src/Complex.cpp SOLID-2.0/src/BBoxTree.cpp
		  SOLID-2.0/src/Endpoint.cpp SOLID-2.0/src/Object.cpp
		  SOLID-2.0/src/Response.cpp SOLID-2.0/src/RespTable.cpp
		  SOLID-2.0/src/C-api.cpp)

INCLUDE_DIRECTORIES(SOLID-2.0/include)

IF(UNIX)
	ADD_LIBRARY(SOLID-2.0/src/libsolid ${SOLID_SOURCES})
	SET_TARGET_PROPERTIES(SOLID-2.0/src/libsolid PROPERTIES PREFIX ""
		COMPILE_FLAGS -fPIC)
ENDIF(UNIX)

#ELSE(UNIX)
#	ADD_LIBRARY(SOLID-2.0/src/solid ${SOLID_SOURCES})
#	SET_TARGET_PROPERTIES(SOLID-2.0/src/solid PROPERTIES COMPILE_FLAGS -fPIC)
#ENDIF(UNIX)


SET(SIMUV2.1_SOURCES aero.cpp axle.cpp brake.cpp car.cpp categories.cpp
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
	ADD_LIBRARY(simuv2.1 SHARED ${SIMUV2.1_SOURCES} simuv2.1.def)
ELSE(WIN32)
	ADD_LIBRARY(simuv2.1 SHARED ${SIMUV2.1_SOURCES})
ENDIF(WIN32)

IF(UNIX)
	SET_TARGET_PROPERTIES(simuv2.1 PROPERTIES PREFIX "")
ENDIF(UNIX)

ADD_TORCSLIB_LIBRARY(simuv2.1 tgf robottools solid)

TORCS_INSTALL_FILES(LIB modules/simu TARGETS simuv2.1)

