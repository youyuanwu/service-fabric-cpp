set (CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS program_options)


add_library(kvstore_lib)
target_sources(kvstore_lib PRIVATE FILE_SET cxx_modules TYPE CXX_MODULES FILES 
    kvstore.cppm
    kvstore-curd.cppm
    kvstore-transport.cppm
)

target_link_libraries(kvstore_lib 
    PUBLIC fabric_ext
    PUBLIC fabric_internal_ext
    PUBLIC fabric_error
    PUBLIC FabricRuntime
    PUBLIC FabricTransport
    PUBLIC Boost::program_options
)

set(_exe_name kvstore)
add_executable(${_exe_name} kvstore_main.cpp)
target_link_libraries(${_exe_name} PRIVATE kvstore_lib)

set(_pkg_root ${CMAKE_BINARY_DIR}/kvstore_root)

# package the app
add_custom_command(TARGET ${_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_pkg_root}
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/manifests/ApplicationManifest.xml ${_pkg_root}
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/manifests/ServiceManifest.xml ${_pkg_root}/KvStoreServicePackage/ServiceManifest.xml
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different $<TARGET_FILE:${_exe_name}>  ${_pkg_root}/KvStoreServicePackage/Code/${_exe_name}.exe
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${_exe_name}.exe.cfg ${_pkg_root}/KvStoreServicePackage/Code
)

# client exe
add_executable(kvclient kvclient.cpp)
target_link_libraries(kvclient PRIVATE kvstore_lib)