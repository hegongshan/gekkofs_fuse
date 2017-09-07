cmake_minimum_required(VERSION 3.7)
project(ifs)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --pedantic -g -pg")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

set(CMAKE_EXPORT_COMPILE_COMMANDS 0)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
#find_package(CCI REQUIRED)
find_package(BMI REQUIRED)
find_package(Mercury REQUIRED)
find_package(MercuryUtil REQUIRED)
find_package(Abt REQUIRED)
find_package(Abt-Snoozer REQUIRED)
find_package(Margo REQUIRED)

# boost dependencies, system is required for filesystem #TODO VERSION UNTESTED. I USE 1.62
find_package(Boost 1.58 REQUIRED COMPONENTS system filesystem serialization)

include_directories(include ${ROCKSDB_INCLUDE_DIR}
        # margo paths
        ${MARGO_INCLUDE_DIR} ${ABT_INCLUDE_DIR} ${ABT_SNOOZER_INCLUDE_DIR} ${MERCURY_INCLUDE_DIR} ${LIBEV_INCLUDE_DIRS} ${ABT_IO_INCLUDE_DIRS}
        )

include_directories(include)

set(SOURCE_FILES main.cpp main.hpp configure.hpp util.cpp
        src/db/db_util.cpp src/classes/fs_data.cpp src/classes/rpc_data.cpp

        include/db/db_util.hpp include/classes/fs_data.hpp include/classes/rpc_data.hpp
        include/classes/metadata.hpp src/classes/metadata.cpp
        src/daemon/adafs_daemon.cpp include/daemon/adafs_daemon.hpp

        include/rpc/rpc_defs.hpp include/rpc/rpc_types.hpp


        src/daemon/fs_operations.cpp src/daemon/fs_operations.cpp include/daemon/fs_operations.hpp src/adafs_ops/metadentry.cpp include/adafs_ops/metadentry.hpp src/db/db_ops.cpp src/db/db_ops.cpp include/db/db_ops.hpp src/rpc/handler/h_metadentry.cpp src/rpc/sender/c_metadentry.cpp include/rpc/sender/c_metadentry.hpp)
add_executable(adafs_daemon ${SOURCE_FILES})
target_link_libraries(adafs_daemon ${ROCKSDB_LIBRARIES}
        # rocksdb libs
        ${snappy_LIBRARIES} ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${ZSTD_LIBRARIES} ${gflags_LIBRARIES} ${LZ4_LIBRARY}
        # margo libs
        ${BMI_LIBRARIES} ${MERCURY_LIBRARIES} ${MERCURY_UTIL_LIBRARIES} ${ABT_LIBRARIES} ${ABT_SNOOZER_LIBRARIES} ${MARGO_LIBRARIES}
        -lpthread -lboost_system -lboost_filesystem -lboost_serialization -lboost_program_options -pg)

add_subdirectory(src/preload)