INCLUDE(../../../cmake/macros.cmake)

# The ubiquitous robot module and its clones.
SET(ROBOT_NAME "shadow")

# Official-only shadow interface.
SET(ROBOT_INTERFACE LEGACY WELCOME)

SET(ROBOT_SOURCES 
	src/ClothoidPath.cpp
    src/ClothoidPath.h
    src/cubic.cpp
    src/cubic.h
    src/danpath.cpp
    src/danpath.h
    src/driver.cpp
    src/driver.h
    src/globaldefinitions.h
    src/LinePath.cpp
    src/LinePath.h
    src/MyTrack.cpp
    src/MyTrack.h
    src/opponent.cpp
    src/opponent.h
    src/pidcontroller.cpp
    src/pidcontroller.h
    src/pit.cpp
    src/pit.h
    src/Seg.h
    src/Shadow.cpp
    src/spline.cpp
    src/spline.h
    #src/Strategy.cpp
    #src/Strategy.h
    src/torcs_or_sd.h
    src/Utils.cpp
    src/Utils.h
    src/Vec2d.h
    src/Vec3d.h)

# Official-only shadow instances.
SET(ROBOT_CLONES shadow_36GP shadow_67GP shadow_sc shadow_srw shadow_trb1 shadow_ls1 shadow_ls2 shadow_mp5
			shadow_mpa1 shadow_mpa11 shadow_mpa12 shadow_lp1 shadow_ref)

# The ubiquitous robot module and its clones.
ROBOT_MODULE(NAME ${ROBOT_NAME} VERSION 3.3.0 SOVERSION 1.0.0
             INTERFACE ${ROBOT_INTERFACE}
             SOURCES ${ROBOT_SOURCES}
             CLONENAMES ${ROBOT_CLONES})

# For data associated to the ubiquitous robot module and its clones,
# see data/drivers/shadow*/CMakeLists.txt
