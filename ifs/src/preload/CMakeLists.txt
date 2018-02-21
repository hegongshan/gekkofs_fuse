cmake_minimum_required(VERSION 3.6)
project(preload CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall --pedantic -g -pg -fPIC")


set(SOURCE_FILES_PRELOAD preload.cpp ../../include/configure.hpp ../../configure_public.hpp
        open_file_map.cpp ../../include/preload/preload.hpp
        ../../include/preload/open_file_map.hpp preload_util.cpp ../../include/preload/preload_util.hpp

        ../../include/preload/ipc_types.hpp margo_ipc.cpp ../../include/preload/margo_ipc.hpp

        rpc/ld_rpc_data_ws.cpp rpc/ld_rpc_metadentry.cpp ../../include/preload/rpc/ld_rpc_data_ws.hpp ../../include/preload/rpc/ld_rpc_metadentry.hpp
        ../../include/rpc/rpc_utils.hpp ../rpc/rpc_utils.cpp intcp_functions.cpp passthrough.cpp ../../include/preload/passthrough.hpp ../../include/global_defs.hpp adafs_functions.cpp adafs_functions.cpp ../../include/preload/adafs_functions.hpp)
add_library(iointer SHARED ${SOURCE_FILES_PRELOAD})
target_link_libraries(iointer dl ${NA_LIB} ${MERCURY_LIBRARIES} ${MERCURY_UTIL_LIBRARIES} ${ABT_LIBRARIES}
        ${ABT_SNOOZER_LIBRARIES} ${MARGO_LIBRARIES}
        -lpthread -lboost_system)