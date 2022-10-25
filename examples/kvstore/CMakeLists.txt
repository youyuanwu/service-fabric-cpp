set (CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES
*.ixx
*.cpp
)

set(_exe_name kvstore)
add_executable(${_exe_name} ${SOURCES})

target_link_libraries(${_exe_name}  
    PRIVATE fabric_ext
    PRIVATE fabric_internal_ext
    PRIVATE fabric_error
    PRIVATE FabricRuntime
    PRIVATE FabricTransport
)

target_compile_definitions(${_exe_name} 
    PRIVATE _WIN32_WINNT=0x0601
)

target_compile_options(${_exe_name}
    PUBLIC /experimental:module
    PUBLIC /std:c++latest
)

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
