cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(pack LANGUAGES CXX)

add_executable(Tests tests/tests.cpp)
target_include_directories(Tests PRIVATE "include/")