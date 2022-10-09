cmake_minimum_required(VERSION 3.14)

project(service-fabric)

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

add_subdirectory(libs)

if(service_fabric_BuildExamples)
  add_subdirectory(examples)
endif()