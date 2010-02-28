INCLUDE(../../../cmake/macros.cmake)
PROJECT(accc)

SET(ACCC_SOURCES ac3dgroup.cpp  ac3dload.cpp  mainaccc.cpp)

ADD_TORCSLIB_INCLUDEDIR(portability)

ADD_EXECUTABLE(sd-accc ${ACCC_SOURCES})

TORCS_INSTALL_FILES(BIN TARGETS sd-accc)

