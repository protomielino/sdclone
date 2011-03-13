INCLUDE("../../../../cmake/macros.cmake")

SET(SIMUV3_SOURCES SimulationOptions.cpp aero.cpp axle.cpp brake.cpp
                   car.cpp categories.cpp collide.cpp differential.cpp
                   engine.cpp simu.cpp simuitf.cpp steer.cpp susp.cpp
                   transmission.cpp wheel.cpp)

ADD_INTERFACE_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math tgf robottools portability)
INCLUDE_DIRECTORIES(../simuv2.1/SOLID-2.0/include)

IF(WIN32)
	# Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

IF(WIN32)
	ADD_LIBRARY(simuv3 SHARED ${SIMUV3_SOURCES} simuv3.def)
ELSE(WIN32)
	ADD_LIBRARY(simuv3 SHARED ${SIMUV3_SOURCES})
ENDIF(WIN32)

ADD_SDLIB_LIBRARY(simuv3 tgf robottools solid)

SET_TARGET_PROPERTIES(simuv3 PROPERTIES PREFIX "")

SD_INSTALL_FILES(LIB modules/simu TARGETS simuv3)

