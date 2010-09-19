INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "human")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp pref.cpp human.h pref.h)

SET(ROBOT_SHIP logo.rgb)
SET(ROBOT_SHIPSUBDIRS cars)

FILE(GLOB files_xml *.xml)

ADD_SDLIB_INCLUDEDIR(tgfclient)

ROBOT(NAME ${ROBOT_NAME} INTERFACE WELCOME SOURCES ${ROBOT_SOURCES} 
      SHIP ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

SD_INSTALL_FILES(DATA drivers/${ROBOT_NAME} USER drivers/${ROBOT_NAME}
	                  FILES ${files_xml})

#ADD_SDLIB_LIBRARY(${ROBOT_NAME} tgf tgfclient)
ADD_SDLIB_LIBRARY(${ROBOT_NAME} tgf client)
