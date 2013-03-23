INCLUDE(../../../cmake/macros.cmake)

# The template robot module.
SET(_NAME "template")

SET(_SOURCES 
    robotmodule.cpp
    robotmodule.h
    driverset.cpp
    driverset.h
    driver.cpp
    driver.h
)

# The ubiquitous robot module and its clones.
ROBOT_MODULE(NAME ${_NAME} VERSION 1.0.0 SOVERSION 1.0.0
             SOURCES ${_SOURCES})

# For data associated to the multi-driverset robot module,
# see data/drivers/template*/CMakeLists.txt
