FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.3.2 # or a later release
)

file(COPY resources DESTINATION .)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/test *.cpp)
add_executable(parblo_tests ${TEST_SOURCES})

target_link_libraries(parblo_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(parblo_tests PUBLIC parblo)

include(CTest)
include(Catch)
catch_discover_tests(parblo_tests)
