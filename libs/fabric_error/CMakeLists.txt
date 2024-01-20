# need magic enum for reflection
set (CMAKE_CXX_STANDARD 20)

find_package(magic_enum CONFIG REQUIRED)

add_library(fabric_error STATIC
  include/servicefabric/fabric_error.hpp
  fabric_error.cpp
)
target_include_directories(fabric_error
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(fabric_error
  PRIVATE magic_enum::magic_enum
  PRIVATE fabric_sdk
)