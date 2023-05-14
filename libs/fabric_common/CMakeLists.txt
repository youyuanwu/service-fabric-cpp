# code extracted from sf repo for easy reuse.

set (CMAKE_CXX_STANDARD 23) # for stacktrace

file(GLOB_RECURSE SOURCES
*.cpp
*.h
)


add_library(fabric_common STATIC ${SOURCES})
target_include_directories(fabric_common 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(fabric_common
  PUBLIC fabric_sdk
)