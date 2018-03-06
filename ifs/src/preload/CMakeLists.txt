# Compiler flags for various cmake build types
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fPIC")
set(CMAKE_CXX_FLAGS_MEMCHECK "-Wall --pedantic -g -O0 -fsanitize=address -fno-omit-frame-pointer -fPIC")
set(CMAKE_CXX_FLAGS_MAINTAINER "-Wall --pedantic -g -O0 -pg -no-pie -fPIC") # -no-pie to be tested with gprof
message("* Current library build type is : ${CMAKE_BUILD_TYPE}")

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

if (${CMAKE_BUILD_TYPE} STREQUAL "Maintainer")
    target_link_libraries(iointer dl ${NA_LIB} ${MERCURY_LIBRARIES} ${MERCURY_UTIL_LIBRARIES} ${ABT_LIBRARIES}
            ${ABT_SNOOZER_LIBRARIES} ${MARGO_LIBRARIES}
            -lpthread -lboost_system -pg)
else ()
    target_link_libraries(iointer dl ${NA_LIB} ${MERCURY_LIBRARIES} ${MERCURY_UTIL_LIBRARIES} ${ABT_LIBRARIES}
            ${ABT_SNOOZER_LIBRARIES} ${MARGO_LIBRARIES}
            -lpthread -lboost_system)
endif ()
