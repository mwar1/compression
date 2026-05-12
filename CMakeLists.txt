cmake_minimum_required(VERSION 3.10)

project(CSN
    VERSION 0.3.0 
    LANGUAGES CXX
)

configure_file(
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/generated/version.h"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE IMPL_SOURCES "compressor_impls/*.cpp")

add_library(compression_core STATIC 
    compressor.cpp 
    ${IMPL_SOURCES}
)

target_include_directories(compression_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/generated
)

# Standard executable
add_executable(compressor main.cpp)
if(MSVC)
    target_link_libraries(compressor PRIVATE -WHOLEARCHIVE:compression_core)
else()
    target_link_libraries(compressor PRIVATE -Wl,--whole-archive compression_core -Wl,--no-whole-archive)
endif()

# Testing suite
add_executable(run_tests test.cpp)
if(MSVC)
    target_link_libraries(run_tests PRIVATE -WHOLEARCHIVE:compression_core)
else()
    target_link_libraries(run_tests PRIVATE -Wl,--whole-archive compression_core -Wl,--no-whole-archive)
endif()
