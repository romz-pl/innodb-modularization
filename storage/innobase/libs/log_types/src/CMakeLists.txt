set( LIB_NAME log_types )

add_library( ${LIB_NAME} STATIC
    LSN_PF.cpp
    Link_buf.cpp
    Log_clock.cpp
    Log_clock_point.cpp
    Log_handle.cpp
    atomic_lsn_t.cpp
    atomic_sn_t.cpp
    checkpoint_no_t.cpp
    flags.cpp
    log_block_calc_checksum.cpp
    log_block_calc_checksum_crc32.cpp
    log_block_calc_checksum_none.cpp
    log_block_get_checksum.cpp
    log_block_set_checksum.cpp
    log_checksum_algorithm_ptr.cpp
    log_checksum_func_t.cpp
    log_files_header_fill.cpp
    log_flushes_t.cpp
    log_get_checkpoint_lsn.cpp
    log_get_lsn.cpp
    log_header_format_t.cpp
    log_lock_no_t.cpp
    log_state_t.cpp
    log_sys.cpp
    log_t.cpp
    log_translate_sn_to_lsn.cpp
    lsn_t.cpp
    mlog_id_t.cpp
    recv_recovery_is_on.cpp
    recv_recovery_on.cpp
    sn_t.cpp
    log_writer_mutex_enter.cpp
    log_writer_mutex_exit.cpp
    log_checkpointer_mutex_enter.cpp
    log_checkpointer_mutex_exit.cpp
    log_checkpointer_mutex_own.cpp
    log_writer_mutex_own.cpp
    log_files_real_offset.cpp
    log_files_size_offset.cpp
    log_files_real_offset_for_lsn.cpp
    log_buffer_x_lock_enter.cpp
    log_buffer_x_lock_exit.cpp
    LOG_SYNC_POINT.cpp
    log_test.cpp
    Log_test.cpp
    log_lsn_validate.cpp
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
    crc32
    disk
    sync_rw
    PRIVATE
)

