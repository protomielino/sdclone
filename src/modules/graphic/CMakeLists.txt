INCLUDE(../../../cmake/macros.cmake)

ADD_SUBDIRECTORY(ssggraph)

SET(OPTION_OSG false CACHE BOOL "Enable OpenSceneGraph")
IF(OPTION_OSG)
	ADD_SUBDIRECTORY(osggraph)
ENDIF(OPTION_OSG)

