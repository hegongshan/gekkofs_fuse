cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(lfs VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --pedantic -g -pg")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


# required packages
find_package(FUSE3 REQUIRED)
# Rocksdb dependencies
find_package(LZ4 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(GFlags REQUIRED)
find_package(snappy REQUIRED)
find_package(ZStd REQUIRED)
find_package(RocksDB REQUIRED)
# boost dependencies, system is required for filesystem #TODO VERSION UNTESTED. I USE 1.62
find_package(Boost 1.58 REQUIRED COMPONENTS system filesystem serialization)

include_directories(${FUSE3_INCLUDE_DIR} ${ROCKSDB_INCLUDE_DIR} include/)
set(SOURCE_FILES src/main.cpp src/main.hpp src/fuse_ops.hpp src/configure.hpp
        src/classes/metadata.h src/classes/metadata.cpp src/classes/fs_data.h src/classes/fs_data.cpp
        src/classes/dentry.h src/classes/dentry.cpp

        src/adafs_ops/mdata_ops.hpp src/adafs_ops/dentry_ops.hpp src/adafs_ops/access.hpp

        src/db/db_ops.hpp src/db/db_txn_ops.hpp src/db/util.hpp

        src/util.cpp
        src/fuse_ops/file.cpp src/fuse_ops/directory.cpp src/fuse_ops/sync.cpp src/fuse_ops/access.cpp
        src/adafs_ops/mdata_ops.cpp src/adafs_ops/dentry_ops.cpp src/adafs_ops/access.cpp

        src/db/db_ops.cpp src/db/db_txn_ops.cpp src/db/util.cpp

        )
add_executable(adafs ${SOURCE_FILES} src/main.cpp)
target_link_libraries(adafs ${FUSE3_LIBRARIES} ${ROCKSDB_LIBRARIES}
        ${snappy_LIBRARIES} ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${ZSTD_LIBRARIES} ${gflags_LIBRARIES} ${LZ4_LIBRARY}
        -lpthread -lboost_system -lboost_filesystem -lboost_serialization -pg)
