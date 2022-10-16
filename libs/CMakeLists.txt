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

# main extension lib
add_subdirectory(fabric_ext)
# magic error string lib
add_subdirectory(fabric_error)