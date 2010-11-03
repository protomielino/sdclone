INCLUDE(../../../cmake/macros.cmake)

ADD_SUBDIRECTORY(simuv2.1)
ADD_SUBDIRECTORY(simuv3)

# Should normally becomes obsolete in 2.0.
ADD_SUBDIRECTORY(simuv2)

# Unofficial / obsolete simulation engines.
IF(NOT OPTION_OFFICIAL_ONLY)

	#ADD_SUBDIRECTORY(simuv2)

ENDIF()

