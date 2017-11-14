# Try to find RocksDB headers and library.
#
# Usage of this module as follows:
#
#     find_package(RocksDB)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  ROCKSDB_ROOT_DIR          Set this variable to the root installation of
#                            RocksDB if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  ROCKSDB_FOUND               System has RocksDB library/headers.
#  ROCKSDB_LIBRARIES           The RocksDB library.
#  ROCKSDB_INCLUDE_DIRS        The location of RocksDB headers.

find_path(ROCKSDB_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        PATH_SUFFIXES rocksdb
        )

find_path(ROCKSDB_INCLUDE_DIR rocksdb/db.h
        HINTS
        $ENV{HOME}/adafs/install
        ${ROCKSDB_DIR}
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES include
        PATH_SUFFIXES include/rocksdb
        )

find_library(ROCKSDB_LIBRARY rocksdb
        HINTS
        $ENV{HOME}/adafs/install
        ${ROCKSDB_DIR}
        $ENV{HOME}/opt
        /usr
        /usr/local
        /usr/local/adafs
        /opt/
        PATH_SUFFIXES lib
        PATH_SUFFIXES lib/rocksdb
        )

set(ROCKSDB_INCLUDE_DIRS ${ROCKSDB_INCLUDE_DIR})
set(ROCKSDB_LIBRARIES ${ROCKSDB_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RocksDB DEFAULT_MSG
        ROCKSDB_LIBRARY
        ROCKSDB_INCLUDE_DIR
        )

mark_as_advanced(
        ROCKSDB_DIR
        ROCKSDB_LIBRARY
        ROCKSDB_INCLUDE_DIR
)