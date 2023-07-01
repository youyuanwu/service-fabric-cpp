# fabric error parser cmd tool
# convert fabric error number to string
set(Boost_USE_STATIC_LIBS ON) # use static boost
find_package(Boost REQUIRED COMPONENTS program_options)

set(CMAKE_CXX_STANDARD 20)

add_executable(feparse feparse.cpp)

target_link_libraries(feparse PRIVATE
  fabric_error
  Boost::program_options
)