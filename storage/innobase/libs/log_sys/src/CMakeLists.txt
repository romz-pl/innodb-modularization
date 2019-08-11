set( LIB_NAME log_sys )

add_library( ${LIB_NAME} STATIC
    log_start.cpp
    log_stop_background_threads.cpp
    log_sys_close.cpp
    log_sys_init.cpp
    log_sys_object.cpp
    log_write_ahead_resize.cpp
    log_allocate_recent_written.cpp
    log_allocate_write_ahead_buffer.cpp
    log_allocate_write_events.cpp

    log_deallocate_recent_written.cpp
    log_deallocate_write_ahead_buffer.cpp
    log_deallocate_write_events.cpp
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
    log_closer
    PRIVATE
)

