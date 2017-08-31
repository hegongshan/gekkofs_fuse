cmake_minimum_required(VERSION 3.7)
project(ifs_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(SOURCE_FILES main.cpp)
add_executable(ifs_test ${SOURCE_FILES})