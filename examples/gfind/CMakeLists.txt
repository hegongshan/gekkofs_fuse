set (CMAKE_CXX_STANDARD 14)
add_executable(sfind sfind.cpp)

if(GKFS_INSTALL_TESTS)
    install(TARGETS sfind 
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
