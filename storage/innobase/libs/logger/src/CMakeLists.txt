set( LIB_NAME logger )

add_library( ${LIB_NAME} STATIC
    error_or_warn.cpp
    error.cpp
    fatal_or_error.cpp
    fatal.cpp
    info.cpp
    logger.cpp
    srv_get_server_errmsgs.cpp
    trace_1.cpp
    trace_2.cpp
    trace_3.cpp
    warn.cpp
)

target_compile_options( ${LIB_NAME} PRIVATE  )

target_include_directories(
    ${LIB_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(
    ${LIB_NAME}
    PUBLIC
    error
    PRIVATE
)

