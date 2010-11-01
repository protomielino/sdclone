INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "networkhuman")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp pref.cpp networkhuman.h pref.h)

ADD_SDLIB_INCLUDEDIR(tgfclient networking)

ROBOT_MODULE(NAME ${ROBOT_NAME}
             INTERFACE WELCOME
             SOURCES ${ROBOT_SOURCES})

ROBOT_DATA(NAME ${ROBOT_NAME}
           FILES ${ROBOT_NAME}.xml preferences.xml car.xml logo.png)
#           SUBDIRS cars PATTERNS *.xml)

# TODO: Check if link to client is really needed.
ADD_SDLIB_LIBRARY(${ROBOT_NAME} tgf client networking)
