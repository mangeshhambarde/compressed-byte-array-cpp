project(takehome)

set(CMAKE_CXX_FLAGS "-O3")

cmake_minimum_required(VERSION 3.0)

add_executable(encode encode.cc)
add_executable(decode decode.cc)
target_sources(encode PRIVATE compressed_byte_array.cc)
target_sources(decode PRIVATE compressed_byte_array.cc)
