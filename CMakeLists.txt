cmake_minimum_required(VERSION 3.16)

project(comet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(COMET_BUILD_TESTS "build comet test" OFF)

add_library(comet STATIC src/comet.cpp)
target_include_directories(comet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (COMET_BUILD_TESTS)
    aux_source_directory(tests/ test_source)
    add_executable(comet_test ${test_source})
    target_link_libraries(comet_test PUBLIC comet)
endif()
