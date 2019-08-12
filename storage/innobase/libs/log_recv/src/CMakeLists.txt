set( LIB_NAME log_recv )

add_library( ${LIB_NAME} STATIC
    MetadataRecover.cpp
    flags.cpp
    psf.cpp
    recv_addr_state.cpp
    recv_addr_t.cpp
    recv_calc_lsn_on_data_add.cpp
    recv_data_t.cpp
    recv_dblwr_t.cpp
    recv_encr_ts_list.cpp
    recv_heap_used.cpp
    recv_is_from_backup.cpp
    recv_is_making_a_backup.cpp
    recv_lsn_checks_on.cpp
    recv_max_page_lsn.cpp
    recv_n_pool_free_frames.cpp
    recv_needed_recovery.cpp
    recv_previous_parsed_rec_is_multi.cpp
    recv_previous_parsed_rec_offset.cpp
    recv_previous_parsed_rec_type.cpp
    recv_report_corrupt_log.cpp
    recv_scan_print_counter.cpp
    recv_sys.cpp
    recv_sys_close.cpp
    recv_sys_create.cpp
    recv_sys_finish.cpp
    recv_sys_resize_buf.cpp
    recv_sys_t.cpp
    recv_sys_var_init.cpp
    recv_t.cpp
    recv_writer_thread_active.cpp
    recv_sys_init.cpp
    recv_sys_empty_hash.cpp
    recv_check_log_header_checksum.cpp
    log_block_checksum_is_ok.cpp
    recv_get_page_map.cpp
    recv_get_rec.cpp
    recv_writer_thread.cpp
    recv_sys_free.cpp
    recv_log_recover_pre_8_0_4.cpp
    recv_find_max_checkpoint.cpp
    recv_read_in_area.cpp
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
    buf_pool
    log_sys
    PRIVATE
)

