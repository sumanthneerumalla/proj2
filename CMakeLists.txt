cmake_minimum_required(VERSION 3.6)
project(proj2)

set(CMAKE_CXX_STANDARD 98)

set(SOURCE_FILES Sally.cpp Sally.h driver2.cpp)
add_executable(proj2 ${SOURCE_FILES})