
#
# check g++ compiler version
#
if( CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0 )
    message( FATAL_ERROR
        "
        You use g++ version ${CMAKE_CXX_COMPILER_VERSION}
        g++ compiler must be version 6.0 or later" )
endif()

#
# check clang++ compiler version
#
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang"  AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0 )
    message( FATAL_ERROR
        "
        You use g++ version ${CMAKE_CXX_COMPILER_VERSION}
        clang++ compiler must be version 5.0 or later" )
endif()

