# setup fabric ext lib
file(GLOB_RECURSE SOURCES
*.cpp
*.hpp
)

# lib containers internal idl helpers
set(_lib_name fabric_internal_ext)

add_library(${_lib_name} STATIC ${SOURCES})
target_include_directories(${_lib_name} 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${_lib_name}
  INTERFACE Boost::disable_autolinking Boost::headers
  PUBLIC    moderncom
  PUBLIC    fabric_internal_headers
  PUBLIC    Boost::log
  PUBLIC    fabric_internal_uuids
  PRIVATE   fabric_ext # for async_contxt
)

target_compile_definitions(${_lib_name}
  PUBLIC WIN32_LEAN_AND_MEAN # This is to get rid of include from fabric of winsock.h
  PUBLIC _WIN32_WINNT=0x601  # This is to pin the log version
)

set_property(TARGET ${_lib_name} PROPERTY CXX_STANDARD 20)