set( LIB_NAME log_redo )

add_library( ${LIB_NAME} STATIC
    log_advance_dirty_pages_added_up_to_lsn.cpp
    log_advance_ready_for_write_lsn.cpp
    log_allocate_recent_closed.cpp
    log_allocate_recent_written.cpp
    log_allocate_write_ahead_buffer.cpp
    log_allocate_write_events.cpp
    log_background_threads_active_validate.cpp
    log_background_threads_inactive_validate.cpp
    log_background_write_threads_active_validate.cpp
    log_calc_concurrency_margin.cpp
    log_calc_max_ages.cpp
    log_closer_mutex_enter.cpp
    log_closer_mutex_exit.cpp
    log_closer_mutex_own.cpp
    log_closer_thread_active_validate.cpp
    log_deallocate_recent_closed.cpp
    log_deallocate_recent_written.cpp
    log_deallocate_write_ahead_buffer.cpp
    log_deallocate_write_events.cpp
    log_free_check.cpp
    log_get_max_modified_age_async.cpp
    log_needs_free_check.cpp
    log_position_collect_lsn_info.cpp
    log_position_lock.cpp
    log_position_unlock.cpp
    log_print.cpp
    log_refresh_stats.cpp
    log_start.cpp
    log_stop_background_threads.cpp
    log_sys_close.cpp
    log_sys_init.cpp
    log_sys_object.cpp
    log_threads_active.cpp
    log_wait_for_space_in_log_recent_closed.cpp
    log_write_ahead_resize.cpp





    log_writer_thread_active_validate.cpp
    meb_log_print_file_hdr.cpp
    pfs.cpp
    srv_log_recent_closed_size.cpp
    srv_log_recent_written_size.cpp
    srv_log_write_ahead_size.cpp
    srv_log_write_events.cpp
    srv_log_write_max_size.cpp
    srv_stats.cpp
    srv_stats_t.cpp
    srv_thread_concurrency.cpp
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
    log_buffer
    log_files
    log_flush
    PRIVATE
)

