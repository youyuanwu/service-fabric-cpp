# setup fabric ext lib
file(GLOB_RECURSE SOURCES
*.cpp
*.hpp
)

set(_lib_name fabric_ext2)


add_library(${_lib_name} STATIC ${SOURCES})
target_include_directories(${_lib_name} 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${_lib_name} 
  PUBLIC fabric_sdk
)

#add_dependencies(fabric_ext fabric_import_libs) # make sure import libs are generated

# target_link_directories(fabric_ext
#   PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../fabric_import_libs
# )

target_compile_definitions(${_lib_name}
  PUBLIC WIN32_LEAN_AND_MEAN # This is to get rid of include from fabric of winsock.h
)

# good practice
target_compile_options(${_lib_name}
  PUBLIC /W4 /WX
)

set_property(TARGET fabric_ext PROPERTY CXX_STANDARD 20)