cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(fs VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --pedantic -g")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
find_package(FUSE3 REQUIRED)

# boost dependencies #TODO VERSION UNTESTED. I USE 1.62
find_package(Boost 1.56.0 COMPONENTS system filesystem serialization)

include_directories(${FUSE3_INCLUDE_DIR} include/)
set(SOURCE_FILES src/main.cpp src/main.h src/fuse_ops.h src/util.cpp src/metadata.h src/metadata.cpp src/metadata_ops.h src/metadata_ops.cpp src/dentry_ops.cpp src/dentry_ops.h)
add_executable(adafs ${SOURCE_FILES} src/main.cpp)
target_link_libraries(adafs ${FUSE3_LIBRARIES} -lpthread -lboost_system -lboost_filesystem -lboost_serialization)