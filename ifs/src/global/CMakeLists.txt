add_library(distributor STATIC)
set_property(TARGET distributor PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(distributor
    PUBLIC
    ${INCLUDE_DIR}/global/rpc/distributor.hpp
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/rpc/distributor.cpp
    )
