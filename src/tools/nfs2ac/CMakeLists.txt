INCLUDE(../../../cmake/macros.cmake)

PROJECT(nfs2ac)

SET(NFS2AC_SOURCES mainnfs2ac.cpp)

ADD_EXECUTABLE(sd-nfs2ac ${NFS2AC_SOURCES})
SD_INSTALL_FILES(BIN TARGETS sd-nfs2ac)
