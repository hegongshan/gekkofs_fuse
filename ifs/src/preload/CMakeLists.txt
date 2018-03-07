set(PRELOAD_SRC
    preload.cpp
    open_file_map.cpp
    preload_util.cpp
    margo_ipc.cpp
    intcp_functions.cpp
    passthrough.cpp
    adafs_functions.cpp
    rpc/ld_rpc_data_ws.cpp
    rpc/ld_rpc_metadentry.cpp
    ../global/rpc/rpc_utils.cpp
    )
set(PRELOAD_HEADERS
    ../../configure_public.hpp
    ../../include/global/configure.hpp
    ../../include/global/global_defs.hpp
    ../../include/global/rpc/rpc_utils.hpp
    ../../include/preload/adafs_functions.hpp
    ../../include/preload/ipc_types.hpp
    ../../include/preload/margo_ipc.hpp
    ../../include/preload/open_file_map.hpp
    ../../include/preload/passthrough.hpp
    ../../include/preload/preload.hpp
    ../../include/preload/preload_util.hpp
    ../../include/preload/rpc/ld_rpc_data_ws.hpp
    ../../include/preload/rpc/ld_rpc_metadentry.hpp
    )

add_library(iointer SHARED ${PRELOAD_SRC} ${PRELOAD_HEADERS})

target_link_libraries(iointer
    dl
    ${ABT_LIBRARIES}
    ${ABT_SNOOZER_LIBRARIES}
    ${NA_LIB}
    ${MERCURY_LIBRARIES}
    ${MERCURY_UTIL_LIBRARIES}
    ${MARGO_LIBRARIES}
)

set_target_properties(iointer PROPERTIES LINK_FLAGS "-lpthread -lboost_system")
