#INCLUDE(../../../cmake/macros.cmake)

ADD_SUBDIRECTORY(standardgame)

# Unofficial race engines.
IF(NOT OPTION_OFFICIAL_ONLY)

   IF(OPTION_PARAM_OPTIMIZER)
      SD_ADD_SUBDIRECTORY(genparoptv1)
   ENDIF()

ENDIF()