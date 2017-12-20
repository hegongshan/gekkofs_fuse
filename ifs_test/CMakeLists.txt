cmake_minimum_required(VERSION 3.7)
project(ifs_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

set(SOURCE_FILES main.cpp)
set(SOURCE_FILES_MPI main_MPI.cpp)
set(SOURCE_FILES_TEMP main_temp_testing.cpp)
add_executable(ifs_test ${SOURCE_FILES})
add_executable(ifs_test_MPI ${SOURCE_FILES_MPI})
add_executable(ifs_test_temp ${SOURCE_FILES_TEMP})

target_link_libraries(ifs_test_MPI ${MPI_CXX_LIBRARIES})