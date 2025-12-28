set (CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES
*.cpp
)

foreach(exe_file ${SOURCES})
    get_filename_component(exe_name ${exe_file} NAME_WE)
    add_executable(${exe_name} ${exe_file})
    set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 20)
    target_link_libraries(${exe_name} 
        PRIVATE fabric_ext fabric_error FabricClient 
        argparse::argparse
        spdlog::spdlog
    )
endforeach()