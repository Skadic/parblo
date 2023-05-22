[![CMake](https://github.com/Skadic/parblo/actions/workflows/build.yml/badge.svg)](https://github.com/Skadic/parblo/actions/workflows/build.yml)
# parblo

Block Trees constructed in parallel, written in C++20. This is a heavy WIP and serves as the implementation for my master's thesis.

## Usage

Include this github repository either as a submodule, or by using FetchContent:
```cmake
FetchContent_Declare(
    parblo
    GIT_REPOSITORY https://github.com/Skadic/parblo.git
    GIT_TAG        9ac7e685daf90e6835c383b07ed788fe2a238876 # You probably want a different commit hash than this
)
```

Then, just add `parblo` as a library.
```cmake
target_link_libraries(my_target INTERFACE parblo)
```

## Building

To build the tests in this library, run:
```sh
cmake --preset default
cmake --build --preset default
```
This results in a binary `/build/relwithdebinfo/test/tests`.
Either run this executable or run
```sh
ctest --preset default
```
to run the tests.

Alternatively, you can run
```sh
cmake --workflow --preset default
```
to configure, build and run tests in one command.

