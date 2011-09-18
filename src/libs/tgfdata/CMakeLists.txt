INCLUDE(../../../cmake/macros.cmake)

#PROJECT(tgfdata)

ADD_PLIB_INCLUDEDIR()

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(portability txml math tgf)

SET(TGFDATA_HEADERS tgfdata.h cars.h tracks.h racemanagers.h race.h drivers.h)
SET(TGFDATA_SOURCES tgfdata.cpp cars.cpp tracks.cpp racemanagers.cpp race.cpp drivers.cpp
                    ${TGFDATA_HEADERS})

IF(WIN32)
    # DLL export stuff under Windows (to avoid .def file)
    ADD_DEFINITIONS(-DTGFDATA_DLL)
	
    # Ignore some run-time libs to avoid link time warnings and sometimes even crashes.
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib")
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(tgfdata SHARED ${TGFDATA_SOURCES})

IF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)
    ADD_LIBRARY(tgfdata_static STATIC ${TGFDATA_SOURCES})
ENDIF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)

ADD_SDLIB_LIBRARY(tgfdata txml tgf)

ADD_PLIB_LIBRARY(tgfdata sg ul)

IF(WIN32)
    SD_INSTALL_FILES(BIN TARGETS tgfdata)
ELSE(WIN32)
    SD_INSTALL_FILES(LIB lib TARGETS tgfdata)
ENDIF(WIN32)

SD_INSTALL_FILES(INCLUDE FILES ${TGFDATA_HEADERS})

