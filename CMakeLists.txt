cmake_minimum_required(VERSION 3.14)

project(service-fabric)

set(service_fabric_MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(service_fabric_MAIN_PROJECT ON)
endif()

option(service_fabric_BuildTests     "Build the unit tests when BUILD_TESTING is enabled." ${service_fabric_MAIN_PROJECT})
option(service_fabric_BuildExamples  "Build examples"                                      ${service_fabric_MAIN_PROJECT})
option(service_fabric_BuildIDLOnly   "Build IDL and fabric libs only"                                                 OFF)

add_subdirectory(fabric_libs)
add_subdirectory(idls)
if(service_fabric_BuildIDLOnly)
  message(STATUS "build idls only. Skip the rest of cpp lib")
  return()
endif()

add_subdirectory(moderncom)

set(Boost_USE_STATIC_LIBS ON) # use static boost
find_package(Boost REQUIRED COMPONENTS log) # log is used for tests

add_library(servicefabric INTERFACE)
target_include_directories(servicefabric 
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(servicefabric
  INTERFACE Boost::log Boost::disable_autolinking Boost::headers
  INTERFACE moderncom
  INTERFACE servicefabric_static
)

target_compile_definitions(servicefabric
  INTERFACE WIN32_LEAN_AND_MEAN
) # This is to get rid of include from fabric of winsock.h

add_subdirectory(libs/fabric_error)

if(service_fabric_BuildExamples)
  add_subdirectory(examples)
endif()