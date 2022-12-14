# common lib dir
if(NOT ServiceFabric_Runtime_ROOT)
    set(ServiceFabric_Runtime_ROOT "C:\\Program Files\\Microsoft Service Fabric")
endif()

set(ServiceFabric_Runtime_BINARY_DIR "${ServiceFabric_Runtime_ROOT}\\bin\\Fabric\\Fabric.Code")

# add custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${service-fabric_SOURCE_DIR}/cmake")

# import libs generated from dlls
add_subdirectory(fabric_import_libs)
# fetch idl files
add_subdirectory(fabric_idls)

if(service_fabric_BuildIDLOnly)
  message(STATUS "build idls only. Skip the rest of cpp lib")
  return()
endif()

add_subdirectory(fabric_internal)

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

# main extension lib
add_subdirectory(fabric_ext)
# internal extension lib
add_subdirectory(fabric_internal_ext)
# magic error string lib
add_subdirectory(fabric_error)