INCLUDE(../../../cmake/macros.cmake)

#PROJECT(learning)

SET(LEARNING_HEADERS ANN.h ann_policy.h Distribution.h learn_debug.h List.h MathFunctions.h policy.h
                     real.h string_utils.h)
SET(LEARNING_SOURCES ANN.cpp Distribution.cpp List.cpp MathFunctions.cpp
                     ann_policy.cpp policy.cpp string_utils.cpp
		     ${LEARNING_HEADERS})

ADD_TORCSLIB_INCLUDEDIR(portability)

IF(UNIX)
	ADD_LIBRARY(learning SHARED ${LEARNING_SOURCES})
ENDIF(UNIX)
IF(WIN32)
	ADD_LIBRARY(learning STATIC ${LEARNING_SOURCES})
ENDIF(WIN32)


TORCS_INSTALL_FILES(LIB lib TARGETS learning)
TORCS_INSTALL_FILES(INCLUDE learning FILES ${LEARNING_HEADERS})

