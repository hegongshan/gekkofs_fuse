cmake_minimum_required(VERSION 3.6)
project(fuse3 CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --pedantic -g -pg")

# required packages
find_package(FUSE3 REQUIRED)

include_directories(${FUSE3_INCLUDE_DIR})
set(SOURCE_FILES_FUSE fuse_main.cpp
        ../../include/fuse3/fuse_main.hpp ../../include/fuse3/fuse_ops.hpp fuse_util.cpp ../../include/fuse3/fuse_util.hpp)
add_executable(adafs_fuse ${SOURCE_FILES_FUSE})
target_link_libraries(adafs_fuse ${FUSE3_LIBRARIES} ${NA_LIB} ${MERCURY_LIBRARIES} ${MERCURY_UTIL_LIBRARIES}
        ${ABT_LIBRARIES} ${ABT_SNOOZER_LIBRARIES} ${MARGO_LIBRARIES})