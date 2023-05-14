file(GLOB_RECURSE SOURCES
*.cpp
)

set(test_name fabric_ext2_test)
add_executable(${test_name} ${SOURCES})
target_include_directories(${test_name} 
  PRIVATE .
)

target_link_libraries(${test_name} PRIVATE
    PUBLIC Boost::unit_test_framework Boost::disable_autolinking
    fabric_error
    fabric_ext
    FabricClient
    fabric_ext2
)
set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 20)
add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
