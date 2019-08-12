set( LIB_NAME log_types )

add_library( ${LIB_NAME} STATIC
    LOG_SYNC_POINT.cpp
    Link_buf.cpp
    Log_clock.cpp
    Log_clock_point.cpp
    Log_handle.cpp
    Log_test.cpp
    checkpoint_no_t.cpp
    flags.cpp
    log_buffer_x_lock_enter.cpp
    log_buffer_x_lock_exit.cpp
    log_checksum_algorithm_ptr.cpp
    log_checksum_func_t.cpp
    log_flushes_t.cpp
    log_get_checkpoint_lsn.cpp
    log_header_format_t.cpp
    log_lock_no_t.cpp
    log_state_t.cpp
    log_sys.cpp
    log_t.cpp
    log_test.cpp
    log_writer_mutex_enter.cpp
    log_writer_mutex_exit.cpp
    log_writer_mutex_own.cpp
    mlog_id_t.cpp
    recv_recovery_is_on.cpp
    recv_recovery_on.cpp
    log_get_lsn.cpp
    log_update_limits.cpp
    log_free_check_margin.cpp
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
    log_sn
    sync_rw
    PRIVATE
)

