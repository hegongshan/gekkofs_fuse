cmake_minimum_required(VERSION 3.4)
project(stackfs_lowlevel LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -D_FILE_OFFSET_BITS=64")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall --pedantic -g")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
find_package(FUSE REQUIRED)

include_directories(${FUSE_INCLUDE_DIR})
set(SOURCE_FILES main.c)
add_executable(stackfs_lowlevel ${SOURCE_FILES})
target_link_libraries(stackfs_lowlevel ${FUSE_LIBRARIES} -lpthread)