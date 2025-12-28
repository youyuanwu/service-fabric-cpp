# fabric error parser cmd tool
# convert fabric error number to string

set(CMAKE_CXX_STANDARD 20)

add_executable(feparse feparse.cpp)

target_link_libraries(feparse 
  PRIVATE fabric_error argparse::argparse
)