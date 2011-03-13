INCLUDE(../../../cmake/macros.cmake)

#PROJECT(tgf)

ADD_SDL_INCLUDEDIR()

ADD_SDLIB_INCLUDEDIR(txml portability)

SET(TGF_HEADERS os.h tgf.h modinfo.h osspec.h)
SET(TGF_SOURCES directory.cpp file.cpp filesetup.cpp 
                formula.cpp hash.cpp module.cpp modinfo.cpp os.cpp
                params.cpp profiler.cpp schedulespy.cpp tgf.cpp trace.cpp)

IF(WIN32)
    SET(TGF_HEADERS ${TGF_HEADERS} windowsspec.h)
    SET(TGF_SOURCES ${TGF_SOURCES} windowsspec.cpp)
ELSE(WIN32)
    SET(TGF_HEADERS ${TGF_HEADERS} linuxspec.h)
    SET(TGF_SOURCES ${TGF_SOURCES} linuxspec.cpp)
ENDIF(WIN32)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

ADD_LIBRARY(tgf SHARED ${TGF_HEADERS} ${TGF_SOURCES})

IF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)
    ADD_LIBRARY(tgf_static STATIC ${TGF_HEADERS} ${TGF_SOURCES})
ENDIF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)

ADD_SDLIB_LIBRARY(tgf txml)

ADD_SDL_LIBRARY(tgf)

IF(WIN32)
    SD_INSTALL_FILES(BIN TARGETS tgf)
ELSE(WIN32)
    SD_INSTALL_FILES(LIB lib TARGETS tgf)
ENDIF(WIN32)

SD_INSTALL_FILES(INCLUDE FILES ${TGF_HEADERS})

SD_INSTALL_FILES(DATA config FILES params.dtd)
