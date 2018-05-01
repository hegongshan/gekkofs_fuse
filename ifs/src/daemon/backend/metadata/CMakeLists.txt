add_library(metadata_db STATIC)

target_sources(metadata_db
    PUBLIC
    ${INCLUDE_DIR}/daemon/backend/metadata/db.hpp
    ${INCLUDE_DIR}/daemon/backend/exceptions.hpp
    PRIVATE
    ${INCLUDE_DIR}/daemon/backend/metadata/merge.hpp
    ${CMAKE_CURRENT_LIST_DIR}/merge.cpp
    ${CMAKE_CURRENT_LIST_DIR}/db.cpp
    )

target_link_libraries(metadata_db
    ${ROCKSDB_LIBRARIES}
    # rocksdb libs
    ${snappy_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${ZSTD_LIBRARIES}
    ${LZ4_LIBRARY}
    ${gflags_LIBRARIES}
    ${JEMALLOC_LIBRARIES}
    )

target_include_directories(metadata_db
    PRIVATE
    ${ROCKSDB_INCLUDE_DIRS}
    )
