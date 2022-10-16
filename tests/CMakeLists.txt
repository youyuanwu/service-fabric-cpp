# setup boost
set(Boost_USE_STATIC_LIBS ON) # use static boost
find_package(Boost REQUIRED COMPONENTS unit_test_framework ) # log is used for tests

add_subdirectory(fabric_tests)
add_subdirectory(service_communication)
add_subdirectory(fabric_transport)