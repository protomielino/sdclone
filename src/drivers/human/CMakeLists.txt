INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "human")

SET(ROBOT_SOURCES ${ROBOT_NAME}.cpp pref.cpp human.h pref.h)

ADD_SDLIB_INCLUDEDIR(tgfclient)
ADD_SDL_INCLUDEDIR()

ROBOT_MODULE(NAME ${ROBOT_NAME} VERSION ${VERSION} SOVERSION 1.0.0
             INTERFACE WELCOME
             SOURCES ${ROBOT_SOURCES})

# For data associated to the robot module, see data/drivers/human/CMakeLists.txt
