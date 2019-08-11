set( LIB_NAME log_closer )

add_library( ${LIB_NAME} STATIC
    log_closer.cpp
    srv_log_closer_spin_delay.cpp
    srv_log_closer_timeout.cpp

    log_allocate_recent_closed.cpp
    log_deallocate_recent_closed.cpp

    srv_log_recent_closed_size.cpp
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
    log_write
    PRIVATE
)

