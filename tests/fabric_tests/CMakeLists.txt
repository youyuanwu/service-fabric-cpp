file(GLOB SOURCES
*_test.cpp
)

# strip file extension
foreach(test_file ${SOURCES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file})
    target_include_directories(${test_name} 
      PRIVATE .
    )

    target_link_libraries(${test_name} PRIVATE
        PUBLIC Boost::unit_test_framework Boost::disable_autolinking
        fabric_error
        fabric_ext
        FabricClient
        WindowsApp.lib
    )
    set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 20)
    add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach()