cmake_minimum_required(VERSION 3.28)

set(service_fabric_MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(service_fabric_MAIN_PROJECT ON)
endif()

if(service_fabric_MAIN_PROJECT)
    # configure vcpkg
    # no CmakePresets.json since it works not well with vscode yet.
    if ("$ENV{VCPKG_ROOT}" STREQUAL "")
    message(FATAL_ERROR "VCPKG_ROOT not found")
    endif()
    cmake_path(SET _vcpkg_cmake_file "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${_vcpkg_cmake_file}")
    # these vars are not working
    # set(VCPKG_TARGET_ARCHITECTURE "x64")
    # set(VCPKG_CRT_LINKAGE "static")
    # set(VCPKG_LIBRARY_LINKAGE "static")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static")
    # Set MSVC runtime to MultiThreaded (/MT) static crt
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(service-fabric LANGUAGES C CXX)

message(STATUS "vcpkg installed dir: ${VCPKG_INSTALLED_DIR}")

# hack
set(Boost_NO_WARN_NEW_VERSIONS 1)
set(Boost_USE_STATIC_RUNTIME OFF)

option(service_fabric_BuildTests     "Build the unit tests when BUILD_TESTING is enabled." ${service_fabric_MAIN_PROJECT})
option(service_fabric_BuildExamples  "Build examples"                                      ${service_fabric_MAIN_PROJECT})
option(service_fabric_BuildIDLOnly   "Build IDL and fabric libs only"                                                 OFF)

# format
if(${service_fabric_MAIN_PROJECT})
include(cmake/clang-format.cmake)
endif(${service_fabric_MAIN_PROJECT})

# This is to pin the log version
add_compile_definitions(_WIN32_WINNT=0x0602)

message(STATUS "fetching fabric_metadata")
include(FetchContent)
FetchContent_Declare(fabric_metadata
    GIT_REPOSITORY https://github.com/youyuanwu/service-fabric-metadata-fork
    GIT_TAG b5a1b4190cabe3b1bc94ab2b47abaef7121742c5 # special commit with fix.
)
FetchContent_GetProperties(fabric_metadata)
if(NOT fabric_metadata_POPULATED)
    FetchContent_Populate(fabric_metadata)
    add_subdirectory(${fabric_metadata_SOURCE_DIR} ${fabric_metadata_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


add_subdirectory(libs)

if(service_fabric_BuildExamples)
  add_subdirectory(examples)
endif()

if(service_fabric_BuildTests)
  enable_testing()
  add_subdirectory(tests)
endif()