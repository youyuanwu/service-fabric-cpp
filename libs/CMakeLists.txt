if(service_fabric_BuildIDLOnly)
  message(STATUS "build idls only. Skip the rest of cpp lib")
  return()
endif()

#add_subdirectory(fabric_internal)

add_subdirectory(fabric_common)
# main extension lib
add_subdirectory(fabric_ext)
# internal extension lib
add_subdirectory(fabric_internal_ext)
# magic error string lib
add_subdirectory(fabric_error)
add_subdirectory(fabric_ext2)