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
  PUBLIC fabric_sdk
  PUBLIC    Boost::log
)

#add_dependencies(fabric_ext fabric_import_libs) # make sure import libs are generated

# target_link_directories(fabric_ext
#   PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../fabric_import_libs
# )

target_compile_definitions(fabric_ext
  PUBLIC WIN32_LEAN_AND_MEAN # This is to get rid of include from fabric of winsock.h
  PUBLIC _WIN32_WINNT=0x601  # This is to pin the log version
)

# good practice
target_compile_options(fabric_ext
  PUBLIC /W4 /WX
)

set_property(TARGET fabric_ext PROPERTY CXX_STANDARD 20)