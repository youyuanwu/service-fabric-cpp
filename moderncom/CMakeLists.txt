# setup moderncom lib

#https://github.com/AlexBAV/moderncom.git
message(STATUS "feching moderncom")
include(FetchContent)
FetchContent_Declare(moderncom
  GIT_REPOSITORY https://github.com/AlexBAV/moderncom.git
  GIT_TAG 08fe9b57772eb2b2d869ed6832aee066abfe90c2)
FetchContent_GetProperties(moderncom)
if(NOT moderncom_POPULATED)
  FetchContent_Populate(moderncom)
  # add subdir is outside and after fmt targets.
endif()

add_library(moderncom INTERFACE)
target_include_directories(moderncom INTERFACE
    ${moderncom_SOURCE_DIR}/include
)

target_compile_definitions(moderncom
    INTERFACE BELT_COM_NO_LEAK_DETECTION
)