INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "networkhuman")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp pref.cpp networkhuman.h pref.h)

SET(ROBOT_SHIP logo.rgb)
SET(ROBOT_SHIP_SUBDIR tracks)

FILE(GLOB files_xml *.xml)

ADD_TORCSLIB_INCLUDEDIR(tgfclient networking)

ROBOT(NAME ${ROBOT_NAME} INTERFACE WELCOME SOURCES ${ROBOT_SOURCES} 
      SHIP ${ROBOT_SHIP} SHIPSUBDIRS ${ROBOT_SHIPSUBDIRS})

TORCS_INSTALL_FILES(DATA drivers/${ROBOT_NAME} USER drivers/${ROBOT_NAME}
	                  FILES ${files_xml})

#ADD_TORCSLIB_LIBRARY(${ROBOT_NAME} tgf tgfclient)
ADD_TORCSLIB_LIBRARY(${ROBOT_NAME} tgf client networking)
