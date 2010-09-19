INCLUDE(../../../cmake/macros.cmake)
PROJECT(accc)

SET(ACCC_SOURCES ac3dgroup.cpp  ac3dload.cpp  mainaccc.cpp)

ADD_SDLIB_INCLUDEDIR(portability)

ADD_EXECUTABLE(sd-accc ${ACCC_SOURCES})

SD_INSTALL_FILES(BIN TARGETS sd-accc)

