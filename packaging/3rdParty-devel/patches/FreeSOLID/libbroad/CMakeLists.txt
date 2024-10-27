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
