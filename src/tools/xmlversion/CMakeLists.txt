CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
INCLUDE(../../../cmake/macros.cmake)
PROJECT(xmlversion)

SET(XMLVERSION_SOURCES main.cpp)

ADD_SDLIB_INCLUDEDIR(tgf portability)

IF (COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SD_ADD_EXECUTABLE(xmlversion ${XMLVERSION_SOURCES})

add_subdirectory(
	${TOPLEVEL_DIR}/src/libs/portability
	${CMAKE_BINARY_DIR}/portability
)

add_subdirectory(
	${TOPLEVEL_DIR}/src/libs/tgf
	${CMAKE_BINARY_DIR}/tgf
)

add_subdirectory(
	${TOPLEVEL_DIR}/src/interfaces
	${CMAKE_BINARY_DIR}/interfaces
)

add_subdirectory(
	${TOPLEVEL_DIR}/src/libs/math
	${CMAKE_BINARY_DIR}/math
)

IF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)
	#WARNING: the order of the library in the line below matters! First tgf, then txml!!!
	ADD_SDLIB_LIBRARY(xmlversion portability tgf txml STATIC)
	ADD_SDL2_LIBRARY(xmlversion)
ELSE(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)
	ADD_SDLIB_LIBRARY(xmlversion portability tgf txml)
ENDIF(CMAKE_SKIP_RPATH OR CMAKE_SKIP_BUILD_RPATH)

ADD_DL_LIBRARY(xmlversion)

target_link_libraries(xmlversion portability tgf interfaces math)

install(TARGETS xmlversion DESTINATION bin)
