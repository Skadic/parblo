cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(parblo)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message( FATAL_ERROR "Source builds are prohibited")
endif()

Include(FetchContent)

set(CXX_STANDARD c++20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=gnu++20 ${GCC_WARNINGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native -lprofiler")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -DNDEBUG")

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_COLOR_DIAGNOSTICS ON)
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
  CONFIGURE_COMMAND ""
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

FetchContent_Declare(
  pasta_bit_vector
  GIT_REPOSITORY https://github.com/pasta-toolbox/bit_vector.git
  GIT_TAG 0d3249dc42770a844d56dc25416763d37953aa5c
  BUILD_COMMAND ""
)
FetchContent_MakeAvailable(pasta_bit_vector)
target_link_libraries(parblo INTERFACE pasta_bit_vector)


FetchContent_Declare(
        unordered_dense
        GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
        GIT_TAG e88dd1ce6e9dc5b3fe84a7d93ac1d7f6f7653dbf
        BUILD_COMMAND ""
)
FetchContent_MakeAvailable(unordered_dense)
target_link_libraries(parblo INTERFACE unordered_dense)

# openmp
find_package(OpenMP REQUIRED)
message("OpenMP Version: ${OpenMP_VERSION}")
target_link_libraries(parblo INTERFACE OpenMP::OpenMP_CXX)

# Some compile options
target_compile_options(parblo INTERFACE -Wall -Werror=return-type)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  enable_testing()
  add_subdirectory(test)

  add_executable(build_bt src/build_bt.cpp)
  target_link_libraries(build_bt PRIVATE parblo)
endif()
