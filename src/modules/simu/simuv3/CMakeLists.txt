INCLUDE("../../../../cmake/macros.cmake")

SET(_SOURCES SimulationOptions.cpp aero.cpp axle.cpp brake.cpp
             car.cpp categories.cpp collide.cpp differential.cpp
             engine.cpp simu.cpp steer.cpp susp.cpp
             transmission.cpp wheel.cpp
             aero.h axle.h brake.h carstruct.h differential.h engine.h
             Options.h sim.h steer.h susp.h transmission.h wheel.h
             simuv30.cpp simuv30.h)

ADD_INTERFACE_INCLUDEDIR()
ADD_PLIB_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(math tgf robottools portability)

INCLUDE_DIRECTORIES(../simuv2.1/SOLID-2.0/include)

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DSIMUV30_DLL)

    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

ADD_LIBRARY(simuv3 SHARED ${_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(simuv3 PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

ADD_SDLIB_LIBRARY(simuv3 tgf robottools solid)

IF(UNIX)
    SET_TARGET_PROPERTIES(simuv3 PROPERTIES PREFIX "")
ENDIF(UNIX)

SD_INSTALL_FILES(LIB modules/simu TARGETS simuv3)

