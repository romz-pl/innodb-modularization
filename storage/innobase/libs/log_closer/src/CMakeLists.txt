set( LIB_NAME log_closer )

add_library( ${LIB_NAME} STATIC
    log_advance_dirty_pages_added_up_to_lsn.cpp
    log_allocate_recent_closed.cpp
    log_closer.cpp
    log_closer_mutex_enter.cpp
    log_closer_mutex_exit.cpp
    log_closer_mutex_own.cpp
    log_deallocate_recent_closed.cpp
    srv_log_closer_spin_delay.cpp
    srv_log_closer_timeout.cpp
    srv_log_recent_closed_size.cpp
    log_closer_thread_active_validate.cpp
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

