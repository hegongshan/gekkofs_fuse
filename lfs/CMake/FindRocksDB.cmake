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
        )

find_path(ROCKSDB_INCLUDE_DIR rocksdb/db.h
        HINTS
        /home/evie/adafs/git/rocksdb
        /usr
        /usr/local
        /usr/local/adafs
        ${ROCKSDB_DIR}
        PATH_SUFFIXES include
        )

find_library(ROCKSDB_LIBRARY rocksdb
        HINTS
        /home/evie/adafs/git/rocksdb
        /usr
        /usr/local
        /usr/local/adafs
        ${ROCKSDB_DIR}
        PATH SUFFIXES lib
        PATH_SUFFIXES lib/rocksdb
        #        ${ROCKSDB_ROOT_DIR}/lib
        #        ${ROCKSDB_ROOT_DIR}/lib/rocksdb
        )

set(ROCKSDB_INCLUDE_DIRS ${ROCKSDB_INCLUDE_DIR})
set(ROCKSDB_LIBRARIES ${ROCKSDB_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RocksDB DEFAULT_MSG
        ROCKSDB_INCLUDE_DIR
        ROCKSDB_LIBRARY
        )

mark_as_advanced(
        ROCKSDB_DIR
        ROCKSDB_LIBRARY
        ROCKSDB_INCLUDE_DIR
)