cmake_minimum_required(VERSION 3.24)
project(hashmap)

set(CMAKE_CXX_STANDARD 20)

add_executable(hashmap hash_map.h test_hashmap.cpp)
