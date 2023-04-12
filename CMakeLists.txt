cmake_minimum_required(VERSION 3.16)

project(pack LANGUAGES CXX)

add_executable(Tests tests/tests.cpp)
target_include_directories(Tests PRIVATE "include/")