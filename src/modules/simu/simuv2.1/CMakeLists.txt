IF(NOT OPTION_3RDPARTY_SOLID)

	ADD_SUBDIRECTORY(SOLID-2.0)

ENDIF(NOT OPTION_3RDPARTY_SOLID)

INCLUDE(../../../../cmake/macros.cmake)

ADD_SDLIB_INCLUDEDIR(portability)

# TODO : See if really needed under Linux (not already done by ADD_SUBDIRECTORY ?).
IF(FALSE AND NOT OPTION_3RDPARTY_SOLID)

	# Bundled SOLID 2 sources (soon replaced by 3rdPaty Free Solid 2)
	SET(SOLID_SOURCES SOLID-2.0/src/Transform.cpp SOLID-2.0/src/Convex.cpp
                  SOLID-2.0/src/Box.cpp SOLID-2.0/src/Cone.cpp SOLID-2.0/src/Cylinder.cpp
                  SOLID-2.0/src/Sphere.cpp SOLID-2.0/src/Simplex.cpp
                  SOLID-2.0/src/Polygon.cpp SOLID-2.0/src/Polyhedron.cpp
                  SOLID-2.0/src/Complex.cpp SOLID-2.0/src/BBoxTree.cpp
                  SOLID-2.0/src/Endpoint.cpp SOLID-2.0/src/Object.cpp
                  SOLID-2.0/src/Response.cpp SOLID-2.0/src/RespTable.cpp
                  SOLID-2.0/src/C-api.cpp)

	# BEFORE in order to prevent Windows 3rdParty/include/SOLID from taking the flag.
	INCLUDE_DIRECTORIES(BEFORE SOLID-2.0/include)

	IF(UNIX)
		ADD_LIBRARY(SOLID-2.0/src/libsolid ${SOLID_SOURCES})
    	SET_TARGET_PROPERTIES(SOLID-2.0/src/libsolid PROPERTIES PREFIX ""
        											 COMPILE_FLAGS -fPIC)
	ENDIF(UNIX)

ENDIF(FALSE AND NOT OPTION_3RDPARTY_SOLID)

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DSIMUV21_DLL)
ENDIF(WIN32)

IF(MSVC)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(MSVC)

SET(SIMU_SOURCES aero.cpp axle.cpp brake.cpp car.cpp categories.cpp
                 collide.cpp differential.cpp engine.cpp simu.cpp
                 steer.cpp susp.cpp transmission.cpp wheel.cpp
                 aero.h axle.h brake.h carstruct.h differential.h engine.h
                 sim.h steer.h susp.h transmission.h wheel.h
                 simuv21.cpp simuv21.h)

IF(NOT OPTION_3RDPARTY_SOLID)
	# BEFORE in order to prevent Windows 3rdParty/include/SOLID from taking the flag.
	INCLUDE_DIRECTORIES(BEFORE SOLID-2.0/include)
ELSE(NOT OPTION_3RDPARTY_SOLID)
	ADD_SOLID_INCLUDEDIR()
ENDIF(NOT OPTION_3RDPARTY_SOLID)
ADD_PLIB_INCLUDEDIR()

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math tgf robottools)

ADD_LIBRARY(simuv2.1 SHARED ${SIMU_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(simuv2.1 PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

ADD_SOLID_LIBRARY(simuv2.1) # Ignored if not OPTION_3RDPARTY_SOLID
ADD_SDLIB_LIBRARY(simuv2.1 tgf robottools solid) # solid ignored if not OPTION_3RDPARTY_SOLID

IF(UNIX OR MINGW)
    SET_TARGET_PROPERTIES(simuv2.1 PROPERTIES PREFIX "")
ENDIF(UNIX OR MINGW)

SD_INSTALL_FILES(LIB modules/simu TARGETS simuv2.1)

