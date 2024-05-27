set (CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES
*.cpp
*.hpp
)

add_executable(echomain ${SOURCES})

target_link_libraries(echomain 
    PRIVATE fabric_ext
    PRIVATE fabric_error
    PRIVATE FabricRuntime    
)

set(_pkg_root ${CMAKE_BINARY_DIR}/echoapp_root)

# package the app
add_custom_command(TARGET echomain POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_pkg_root}
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/manifests/ApplicationManifest.xml ${_pkg_root}
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/manifests/ServiceManifest.xml ${_pkg_root}/EchoAppServicePackage/ServiceManifest.xml
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different $<TARGET_FILE:echomain>  ${_pkg_root}/EchoAppServicePackage/Code/echomain.exe
)
