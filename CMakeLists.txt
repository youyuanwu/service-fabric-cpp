cmake_minimum_required(VERSION 3.14)

project(service-fabric)

# find runtime and sdk location
if(NOT ServiceFabric_Runtime_ROOT)
    set(ServiceFabric_Runtime_ROOT "C:\\Program Files\\Microsoft Service Fabric")
endif()

set(ServiceFabric_Runtime_BINARY_DIR "${ServiceFabric_Runtime_ROOT}\\bin\\Fabric\\Fabric.Code")

add_subdirectory(moderncom)
add_subdirectory(fabric_libs)
add_subdirectory(idls)

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
add_subdirectory(examples)