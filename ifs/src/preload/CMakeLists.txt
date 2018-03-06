set(PRELOAD_SOURCES
    preload.cpp
    open_file_map.cpp
    preload_util.cpp
    margo_ipc.cpp
    intcp_functions.cpp
    passthrough.cpp
    adafs_functions.cpp
    rpc/ld_rpc_data_ws.cpp
    rpc/ld_rpc_metadentry.cpp
    ../rpc/rpc_utils.cpp
)

add_library(iointer SHARED ${PRELOAD_SOURCES})

target_link_libraries(iointer
    dl
    ${NA_LIB}
    ${MERCURY_LIBRARIES}
    ${MERCURY_UTIL_LIBRARIES}
    ${ABT_LIBRARIES}
    ${ABT_SNOOZER_LIBRARIES}
    ${MARGO_LIBRARIES}
)

set_target_properties(iointer PROPERTIES LINK_FLAGS "-lpthread -lboost_system")

target_include_directories(iointer PUBLIC
    ${ABT_INCLUDE_DIRS}
)
