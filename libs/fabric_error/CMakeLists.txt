# need magic enum for reflection
set (CMAKE_CXX_STANDARD 20)

message(STATUS "feching magic_enum")
include(FetchContent)
FetchContent_Declare(magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG 1b1194bcd5e0f62047a43161689bae593b12e607)
FetchContent_GetProperties(magic_enum)
if(NOT magic_enum_POPULATED)
  FetchContent_Populate(magic_enum)
  # add subdir is outside and after fmt targets.
endif()

add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE
    ${magic_enum_SOURCE_DIR}/include
)

add_library(fabric_error STATIC
  include/servicefabric/fabric_error.hpp
  fabric_error.cpp
)
target_include_directories(fabric_error
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(fabric_error
  PRIVATE magic_enum
  PRIVATE fabric_sdk
)