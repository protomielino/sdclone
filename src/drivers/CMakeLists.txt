INCLUDE(../../cmake/macros.cmake)

# Official robots.
ADD_SUBDIRECTORY(human)
ADD_SUBDIRECTORY(networkhuman)
ADD_SUBDIRECTORY(simplix)
ADD_SUBDIRECTORY(usr)

# Work-in-progress / unofficial robots.
IF(NOT OPTION_OFFICIAL_ONLY)

	#ADD_SUBDIRECTORY(bt)
	#ADD_SUBDIRECTORY(hymie)
	#ADD_SUBDIRECTORY(K1999)

ENDIF()
