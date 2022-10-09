# import libs generated from dlls
add_subdirectory(fabric_import_libs)
# fetch idl files
add_subdirectory(fabric_idls)

if(service_fabric_BuildIDLOnly)
  message(STATUS "build idls only. Skip the rest of cpp lib")
  return()
endif()

# main extension lib
add_subdirectory(fabric_ext)
# magic error string lib
add_subdirectory(fabric_error)