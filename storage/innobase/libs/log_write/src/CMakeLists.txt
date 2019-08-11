set( LIB_NAME log_write )

add_library( ${LIB_NAME} STATIC
    Log_write_to_file_requests_monitor.cpp
    compute_how_much_to_write.cpp
    compute_next_write_ahead_end.cpp
    compute_real_offset.cpp
    compute_write_event_slot.cpp
    copy_to_write_ahead_buffer.cpp
    current_file_has_space.cpp
    current_write_ahead_enough.cpp
    log_advance_ready_for_write_lsn.cpp



    log_background_threads_active_validate.cpp
    log_background_threads_inactive_validate.cpp
    log_background_write_threads_active_validate.cpp
    log_buffer_reserve.cpp
    log_calc_concurrency_margin.cpp
    log_calc_max_ages.cpp

    log_file_header_fill_encryption.cpp
    log_files_write_buffer.cpp
    log_free_check.cpp
    log_get_max_modified_age_async.cpp
    log_increase_concurrency_margin.cpp
    log_needs_free_check.cpp
    log_position_collect_lsn_info.cpp
    log_position_lock.cpp
    log_position_unlock.cpp
    log_print.cpp
    log_read_encryption.cpp
    log_refresh_stats.cpp
    log_rotate_encryption.cpp
    log_threads_active.cpp
    log_wait_for_space_after_reserving.cpp
    log_wait_for_space_in_log_buf.cpp
    log_wait_for_space_in_log_recent_closed.cpp
    log_wait_for_write.cpp

    log_write_encryption.cpp
    log_write_notifier.cpp
    log_write_notifier_mutex_enter.cpp
    log_write_notifier_mutex_exit.cpp
    log_write_notifier_mutex_own.cpp
    log_write_up_to.cpp
    log_writer.cpp
    log_writer_mutex_enter_nowait.cpp
    log_writer_thread_active_validate.cpp
    log_writer_wait_on_archiver.cpp
    log_writer_wait_on_checkpoint.cpp
    meb_log_print_file_hdr.cpp
    notify_about_advanced_write_lsn.cpp
    pfs.cpp
    prepare_for_write_ahead.cpp
    prepare_full_blocks.cpp
    recv_no_ibuf_operations.cpp
    redo_rotate_default_master_key.cpp
    srv_log_recent_written_size.cpp
    srv_log_write_ahead_size.cpp
    srv_log_write_events.cpp
    srv_log_write_max_size.cpp
    srv_log_write_notifier_spin_delay.cpp
    srv_log_write_notifier_timeout.cpp
    srv_stats.cpp
    srv_stats_t.cpp
    srv_thread_concurrency.cpp
    start_next_file.cpp
    update_current_write_ahead.cpp
    validate_buffer.cpp
    validate_start_lsn.cpp
    write_ahead_enough.cpp
    write_blocks.cpp
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

