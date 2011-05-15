INCLUDE(../../cmake/macros.cmake)

SET(INTERFACES_HEADERS car.h graphic.h playerpref.h raceman.h
                       replay.h robot.h telemetry.h track.h
                       iraceengine.h iuserinterface.h igraphicsengine.h
                       iphysicsengine.h itrackloader.h)

SD_INSTALL_FILES(INCLUDE FILES ${INTERFACES_HEADERS})
