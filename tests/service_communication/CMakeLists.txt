set (CMAKE_CXX_STANDARD 20)

# setup boost
set(Boost_USE_STATIC_LIBS ON) # use static boost
find_package(Boost REQUIRED COMPONENTS unit_test_framework ) # log is used for tests

set(_test_name service_communication_test)

add_executable(${_test_name} ${_test_name}.cpp)

target_link_libraries(${_test_name}
    PUBLIC Boost::unit_test_framework Boost::disable_autolinking
    fabric_internal_headers
    fabric_internal_uuids
    FabricServiceCommunication
    moderncom
    fabric_error
    fabric_ext
)

target_compile_definitions(${_test_name}
  PUBLIC WIN32_LEAN_AND_MEAN # This is to get rid of include from fabric of winsock.h
  PUBLIC _WIN32_WINNT=0x601  # This is to pin the log version
  PUBLIC SF_DEBUG=1
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

