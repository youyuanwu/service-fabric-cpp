# setup moderncom lib
message(STATUS "feching moderncom")
include(FetchContent)
FetchContent_Declare(moderncom
  GIT_REPOSITORY https://github.com/AlexBAV/moderncom.git
  GIT_TAG 08fe9b57772eb2b2d869ed6832aee066abfe90c2)
FetchContent_GetProperties(moderncom)
if(NOT moderncom_POPULATED)
  FetchContent_Populate(moderncom)
  # add subdir is outside and after fmt targets.
endif()

add_library(moderncom INTERFACE)
target_include_directories(moderncom INTERFACE
    ${moderncom_SOURCE_DIR}/include
)

target_compile_definitions(moderncom
    INTERFACE BELT_COM_NO_LEAK_DETECTION
)

# setup boost
set(Boost_USE_STATIC_LIBS ON) # use static boost
find_package(Boost REQUIRED COMPONENTS log) # log is used for tests

# setup fabric ext lib
file(GLOB_RECURSE SOURCES
*.cpp
*.hpp
)

add_library(fabric_ext STATIC ${SOURCES})
target_include_directories(fabric_ext 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(fabric_ext
  INTERFACE Boost::disable_autolinking Boost::headers
  PUBLIC moderncom
  PUBLIC fabric_headers
  PUBLIC    Boost::log
  PUBLIC    fabric_uuids
  PUBLIC    FabricRuntime
  PUBLIC    FabricClient
  PUBLIC    FabricResources
)

add_dependencies(fabric_ext fabric_import_libs) # make sure import libs are generated

target_link_directories(fabric_ext
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../fabric_import_libs
)

target_compile_definitions(fabric_ext
  PUBLIC WIN32_LEAN_AND_MEAN # This is to get rid of include from fabric of winsock.h
  PUBLIC _WIN32_WINNT=0x601  # This is to pin the log version
)

set_property(TARGET fabric_ext PROPERTY CXX_STANDARD 20)