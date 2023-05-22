cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(parblo)

Include(FetchContent)

set(CXX_STANDARD c++20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=gnu++20 ${GCC_WARNINGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
# Copy compile_commands into the root folder
add_custom_target(
  copy-compile-commands ALL
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E echo "Copied compile_commands.json into root folder"
)

add_library(parblo INTERFACE)
target_include_directories(parblo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ----------- Dependencies -----------
# growt
FetchContent_Declare(
  growt
  GIT_REPOSITORY https://github.com/TooBiased/growt.git
  GIT_TAG 0c1148ebcdfd4c04803be79706533ad09cc81d37
  BUILD_COMMAND ""
)
FetchContent_MakeAvailable(growt)
target_include_directories(parblo INTERFACE ${growt_SOURCE_DIR})

# word-packing
FetchContent_Declare(
  word-packing
  GIT_REPOSITORY https://github.com/pdinklag/word-packing.git
  GIT_TAG c7d86247503b71ea359373aa14b9a64b334cbb4a
  BUILD_COMMAND ""
)
FetchContent_MakeAvailable(word-packing)
target_link_libraries(parblo INTERFACE word-packing)

# openmp
find_package(OpenMP REQUIRED)
target_link_libraries(parblo INTERFACE OpenMP::OpenMP_CXX)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  enable_testing()
  add_subdirectory(test)
endif()
