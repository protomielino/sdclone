include_directories(${PROJECT_SOURCE_DIR}/libmoto)

add_library(broad
  BP_C-api.cpp
  BP_Endpoint.cpp
  BP_Endpoint.h
  BP_Proxy.cpp
  BP_Proxy.h
  BP_Scene.cpp
  BP_Scene.h
)

install(
    TARGETS broad
    ARCHIVE DESTINATION lib${LIB_POSTFIX}
    LIBRARY DESTINATION lib${LIB_POSTFIX}
    RUNTIME DESTINATION bin
)
