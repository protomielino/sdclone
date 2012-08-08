INCLUDE(../../../cmake/macros.cmake)

SET(TELEMETRY_SOURCES telemetryitf.cpp telemetry.cpp)

ADD_INTERFACE_INCLUDEDIR()
ADD_SDLIB_INCLUDEDIR(tgf portability)

ADD_LIBRARY(telemetry SHARED ${TELEMETRY_SOURCES})

# Might not work with GCC 4.5 or + (non-robot modules crash at 1st reload = after 1 dlclose) 
#SET_TARGET_PROPERTIES(telemetry PROPERTIES VERSION ${VERSION} SOVERSION 0.0.0)

IF(UNIX OR MINGW)
	SET_TARGET_PROPERTIES(telemetry PROPERTIES PREFIX "")
ENDIF(UNIX OR MINGW)

ADD_SDLIB_LIBRARY(telemetry tgf)

SD_INSTALL_FILES(LIB modules/telemetry TARGETS telemetry)
