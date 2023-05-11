# generic file filter
file(GLOB_RECURSE ALL_SOURCE_FILES 
    examples/*.cpp
    examples/*.hpp
    examples/*.ixx
    include/*.cpp
    include/*.hpp
    tests/*.cpp
    tests/*.hpp
    tests/*.ixx
    libs/*.cpp
    libs/*.hpp
    libs/*.h
    libs/*.ixx
    src/*.cpp
    src/*.hpp
)

add_custom_target(
  clangformat
  COMMAND clang-format
  -i
  ${ALL_SOURCE_FILES}
)

add_custom_target(
  clangformat-check 
  COMMAND clang-format
  --dry-run --Werror
  ${ALL_SOURCE_FILES}
)

# clang-format -style=llvm -dump-config > .clang-format