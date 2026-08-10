cmake_minimum_required(VERSION 3.22)

project(voy
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VOY_WARNING_FLAGS
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wconversion
  -Wsign-conversion
  -Wnull-dereference
  -Wdouble-promotion
  -Wformat=2
)

add_library(voy_lib STATIC
  src/config.cpp
  src/event.cpp
  src/glob.cpp
)
target_include_directories(voy_lib PUBLIC include)
target_compile_options(voy_lib PRIVATE ${VOY_WARNING_FLAGS})

target_link_libraries(voy_lib
  PUBLIC
    nlohmann_json::nlohmann_json
)

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
)
FetchContent_MakeAvailable(doctest)

enable_testing()

add_executable(voy_tests
  test/test_main.cpp
  test/test_event.cpp
)

target_link_libraries(voy_tests
  PRIVATE
    voy_lib
    doctest::doctest
)

target_compile_options(voy_tests PRIVATE ${VOY_WARNING_FLAGS})

add_test(NAME voy_tests COMMAND voy_tests)
