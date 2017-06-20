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
# margo dependencies
find_package(Libev REQUIRED)
find_package(CCI REQUIRED)
find_package(Mercury REQUIRED)
find_package(MercuryUtil REQUIRED)
find_package(Abt REQUIRED)
find_package(Abt-Snoozer REQUIRED)
find_package(Margo REQUIRED)
# Optional dependencies
find_package(AbtIO)
option(WITH_AbtIO "AbtIO enabled" ON)
if (WITH_AbtIO)
    add_definitions(-DUSE_AbtIO=1)
else ()
    add_definitions(-DUSE_AbtIO=0)
endif ()


# boost dependencies, system is required for filesystem #TODO VERSION UNTESTED. I USE 1.62
find_package(Boost 1.58 REQUIRED COMPONENTS system filesystem serialization)

include_directories(${FUSE3_INCLUDE_DIR} ${ROCKSDB_INCLUDE_DIR} include/
        # margo paths
        ${MARGO_INCLUDE_DIR} ${ABT_INCLUDE_DIR} ${ABT_SNOOZER_INCLUDE_DIR} ${MERCURY_INCLUDE_DIR} ${LIBEV_INCLUDE_DIRS} ${ABT_IO_INCLUDE_DIRS}
        )
set(SOURCE_FILES src/main.cpp src/main.hpp src/fuse_ops.hpp src/configure.hpp
        src/classes/metadata.h src/classes/metadata.cpp src/classes/fs_data.h src/classes/fs_data.cpp
        src/classes/dentry.h src/classes/dentry.cpp
        # adafs_ops header
        src/adafs_ops/mdata_ops.hpp src/adafs_ops/dentry_ops.hpp src/adafs_ops/access.hpp
        # db header
        src/db/db_ops.hpp src/db/db_txn_ops.hpp src/db/db_util.hpp
        # rpc header
        src/rpc/rpcs.hpp src/rpc/rpc_util.hpp

        src/util.cpp
        src/fuse_ops/file.cpp src/fuse_ops/directory.cpp src/fuse_ops/sync.cpp src/fuse_ops/access.cpp
        src/adafs_ops/mdata_ops.cpp src/adafs_ops/dentry_ops.cpp src/adafs_ops/access.cpp
        # db src
        src/db/db_ops.cpp src/db/db_txn_ops.cpp src/db/db_util.cpp
        # rpc src
        src/rpc/rpc_util.cpp src/rpc/rpcs.cpp
        )
add_executable(adafs ${SOURCE_FILES} src/main.cpp)
target_link_libraries(adafs ${FUSE3_LIBRARIES} ${ROCKSDB_LIBRARIES}
        # rocksdb libs
        ${snappy_LIBRARIES} ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${ZSTD_LIBRARIES} ${gflags_LIBRARIES} ${LZ4_LIBRARY}
        # margo libs
        ${MERCURY_LIBRARIES} ${MERCURY_UTIL_LIBRARIES} ${ABT_LIBRARIES} ${ABT_SNOOZER_LIBRARIES} ${ABT_IO_LIBRARIES} ${MARGO_LIBRARIES}
        -lpthread -lboost_system -lboost_filesystem -lboost_serialization -pg)
