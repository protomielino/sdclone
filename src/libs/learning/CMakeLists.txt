INCLUDE(../../../cmake/macros.cmake)

#PROJECT(learning)

SET(LEARNING_HEADERS learning.h ANN.h ann_policy.h Distribution.h learn_debug.h List.h 
		     MathFunctions.h policy.h real.h string_utils.h)
SET(LEARNING_SOURCES ANN.cpp Distribution.cpp List.cpp MathFunctions.cpp
                     ann_policy.cpp policy.cpp string_utils.cpp
		     ${LEARNING_HEADERS})

ADD_TORCSLIB_INCLUDEDIR(portability)

# Disable developer warning
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

IF(WIN32)
	# DLL export stuff under Windows (to avoid .def file)
	ADD_DEFINITIONS(-DLEARNING_DLL)
ENDIF(WIN32)

ADD_LIBRARY(learning SHARED ${LEARNING_SOURCES})

IF(WIN32)
	TORCS_INSTALL_FILES(BIN TARGETS learning)
ELSE(WIN32)
	TORCS_INSTALL_FILES(LIB lib TARGETS learning)
ENDIF(WIN32)

TORCS_INSTALL_FILES(INCLUDE learning FILES ${LEARNING_HEADERS})

