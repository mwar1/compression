cmake_minimum_required(VERSION 3.10)
project(Compressors)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE IMPL_SOURCES "compressor_impls/*.cpp")

add_library(compression_core OBJECT 
    compressor.cpp 
    ${IMPL_SOURCES}
)

target_include_directories(compression_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Standard executable
add_executable(compressor main.cpp)
target_link_libraries(compressor PRIVATE $<TARGET_OBJECTS:compression_core>)

# Testing suite
add_executable(run_tests test.cpp)
target_link_libraries(run_tests PRIVATE $<TARGET_OBJECTS:compression_core>)


# Ensure the test runner can find your headers
target_include_directories(run_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
