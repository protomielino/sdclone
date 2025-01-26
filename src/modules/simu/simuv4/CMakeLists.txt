INCLUDE(../../../../cmake/macros.cmake)

ADD_SDLIB_INCLUDEDIR(portability)

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DSIMUV4_DLL)
ENDIF(WIN32)

IF(MSVC)
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(MSVC)

SET(SIMU_SOURCES aero.cpp atmosphere.cpp axle.cpp brake.cpp car.cpp categories.cpp
                 collide.cpp differential.cpp engine.cpp simu.cpp
                 steer.cpp susp.cpp transmission.cpp wheel.cpp
                 simuv4.cpp)
SET(SIMU_HEADERS aero.h axle.h brake.h carstruct.h differential.h engine.h
                 sim.h steer.h susp.h transmission.h wheel.h
                 simuv4.h)

ADD_PLIB_INCLUDEDIR()

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math tgf robottools)

# Note: Headers needed for having them available in IDEs.
SD_ADD_LIBRARY(simuv4 MODULE ${SIMU_SOURCES} ${SIMU_HEADERS})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose)
#SET_TARGET_PROPERTIES(simuv2.1 PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

ADD_SDLIB_LIBRARY(simuv4 portability tgf robottools solid)

SD_INSTALL_FILES(LIB modules/simu TARGETS simuv4)
