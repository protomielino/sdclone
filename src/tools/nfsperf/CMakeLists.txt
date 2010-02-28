INCLUDE(../../../cmake/macros.cmake)

PROJECT(nfsperf)

SET(NFSPERF_SOURCES mainnfsperf.cpp)
ADD_OSSPEC_INCLUDEDIR()
ADD_TORCSLIB_INCLUDEDIR(tgf)

IF (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

ADD_EXECUTABLE(sd-nfsperf ${NFSPERF_SOURCES})

ADD_TORCSLIB_LIBRARY(sd-nfsperf tgf txml)

IF(UNIX)
  TARGET_LINK_LIBRARIES(sd-nfsperf dl)
ENDIF(UNIX)

TORCS_INSTALL_FILES(BIN TARGETS sd-nfsperf)
