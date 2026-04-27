cmake_minimum_required(VERSION 3.10)
project(Compressors)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE IMPL_SOURCES "compressor_impls/*.cpp")

add_executable(run_tests 
    test.cpp 
    compressor.cpp 
    ${IMPL_SOURCES}
)

target_include_directories(run_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
