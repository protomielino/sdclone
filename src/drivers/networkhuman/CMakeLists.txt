INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "networkhuman")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp pref.cpp networkhuman.h pref.h)

SET(ROBOT_FILES logo.rgb)
SET(ROBOT_SUBDIRS)

FILE(GLOB files_xml *.xml)

ADD_SDLIB_INCLUDEDIR(tgfclient networking)

ROBOT(NAME ${ROBOT_NAME} INTERFACE WELCOME SOURCES ${ROBOT_SOURCES} 
	FILES ${ROBOT_FILES} SUBDIRS ${ROBOT_SUBDIRS})

SD_INSTALL_FILES(DATA drivers/${ROBOT_NAME} USER drivers/${ROBOT_NAME}
	                  FILES ${files_xml})

ADD_SDLIB_LIBRARY(${ROBOT_NAME} tgf client networking)
