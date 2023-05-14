if(service_fabric_BuildIDLOnly)
  message(STATUS "build idls only. Skip the rest of cpp lib")
  return()
endif()

#add_subdirectory(fabric_internal)

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

add_subdirectory(fabric_common)
# main extension lib
add_subdirectory(fabric_ext)
# internal extension lib
add_subdirectory(fabric_internal_ext)
# magic error string lib
add_subdirectory(fabric_error)
add_subdirectory(fabric_ext2)