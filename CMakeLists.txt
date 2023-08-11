cmake_minimum_required(VERSION 3.14)

project(service-fabric LANGUAGES C CXX)

# hack
set(Boost_NO_WARN_NEW_VERSIONS 1)

set(service_fabric_MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(service_fabric_MAIN_PROJECT ON)
endif()

option(service_fabric_BuildTests     "Build the unit tests when BUILD_TESTING is enabled." ${service_fabric_MAIN_PROJECT})
option(service_fabric_BuildExamples  "Build examples"                                      ${service_fabric_MAIN_PROJECT})
option(service_fabric_BuildIDLOnly   "Build IDL and fabric libs only"                                                 OFF)

# format
if(${service_fabric_MAIN_PROJECT})
include(cmake/clang-format.cmake)
endif(${service_fabric_MAIN_PROJECT})

message(STATUS "fetching fabric_metadata")
include(FetchContent)
FetchContent_Declare(fabric_metadata
    GIT_REPOSITORY https://github.com/youyuanwu/fabric-metadata.git
    GIT_TAG 10c17a874c5b801b3b10b855967480e85b391646
)
FetchContent_GetProperties(fabric_metadata)
if(NOT fabric_metadata_POPULATED)
    FetchContent_Populate(fabric_metadata)
    add_subdirectory(${fabric_metadata_SOURCE_DIR} ${fabric_metadata_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


add_subdirectory(libs)

if(service_fabric_BuildExamples)
  add_subdirectory(examples)
endif()

if(service_fabric_BuildTests)
  enable_testing()
  add_subdirectory(tests)
endif()