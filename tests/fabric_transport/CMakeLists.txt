set (CMAKE_CXX_STANDARD 20)

set(_test_name fabric_transport_test)

add_executable(${_test_name} ${_test_name}.cpp)

target_link_libraries(${_test_name}
    FabricTransport
    fabric_error
    fabric_ext
    fabric_internal_ext
)

target_compile_definitions(${_test_name}
  PUBLIC WIN32_LEAN_AND_MEAN # This is to get rid of include from fabric of winsock.h
  # PUBLIC SF_DEBUG=1
)

target_include_directories(${_test_name}
  PRIVATE .
)

# boost log leaks memory as intended, so do not check mem leaks.
add_test(NAME ${_test_name} COMMAND ${_test_name} --detect_memory_leaks=0
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

set_tests_properties(${_test_name} PROPERTIES RUN_SERIAL TRUE)
add_custom_command(TARGET ${_test_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
        -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${_test_name}.exe.cfg  ${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:DEBUG>:Debug>$<$<CONFIG:RELEASE>:Release>
)