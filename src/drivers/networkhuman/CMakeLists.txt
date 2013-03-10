INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "networkhuman")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp pref.cpp networkhuman.h pref.h)

ADD_SDLIB_INCLUDEDIR(tgfclient networking)
ADD_SDL_INCLUDEDIR() 

ROBOT_MODULE(NAME ${ROBOT_NAME} VERSION ${VERSION} SOVERSION 0.0.0
             INTERFACE WELCOME
             SOURCES ${ROBOT_SOURCES})

ADD_SDLIB_LIBRARY(${ROBOT_NAME} tgfclient networking)

# For data associated to the robot module, see data/drivers/networkhuman/CMakeLists.txt