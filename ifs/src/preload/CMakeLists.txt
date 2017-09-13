cmake_minimum_required(VERSION 3.7)
project(preload CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -fPIC")


set(SOURCE_FILES_PRELOAD preload.cpp open_file_map.cpp ../../include/preload/preload.hpp
        ../../include/preload/open_file_map.hpp preload_util.cpp ../../include/preload/preload_util.hpp

        ../../include/preload/ipc_types.hpp margo_ipc.cpp ../../include/preload/margo_ipc.hpp
        )
add_library(iointer SHARED ${SOURCE_FILES_PRELOAD})
target_link_libraries(iointer dl ${BMI_LIBRARIES} ${MERCURY_LIBRARIES} ${MERCURY_UTIL_LIBRARIES} ${ABT_LIBRARIES} ${ABT_SNOOZER_LIBRARIES} ${MARGO_LIBRARIES})