INCLUDE(../../cmake/macros.cmake)

# Official robots.
SD_ADD_SUBDIRECTORY(dandroid)
SD_ADD_SUBDIRECTORY(human)
SD_ADD_SUBDIRECTORY(networkhuman)
SD_ADD_SUBDIRECTORY(shadow)
SD_ADD_SUBDIRECTORY(simplix)
SD_ADD_SUBDIRECTORY(usr)

# Work-in-progress / unofficial robots.
IF(NOT OPTION_OFFICIAL_ONLY)
  # Work-in-progress.
  IF(OPTION_3RDPARTY_SQLITE3)
  	SD_ADD_SUBDIRECTORY(replay)
  ENDIF()
ENDIF()
