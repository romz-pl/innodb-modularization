set( LIB_NAME log_recv )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    recv_data_t.cpp
    recv_t.cpp
    recv_addr_state.cpp
    recv_addr_t.cpp
    recv_dblwr_t.cpp
    MetadataRecover.cpp
    recv_sys_t.cpp
    recv_sys.cpp
    recv_needed_recovery.cpp
    recv_lsn_checks_on.cpp
    recv_n_pool_free_frames.cpp
    recv_encr_ts_list.cpp
    psf.cpp
    recv_is_making_a_backup.cpp
    recv_is_from_backup.cpp
    recv_scan_print_counter.cpp
    recv_previous_parsed_rec_type.cpp
    recv_previous_parsed_rec_offset.cpp
    recv_previous_parsed_rec_is_multi.cpp
    recv_max_page_lsn.cpp
    recv_writer_thread_active.cpp
    recv_calc_lsn_on_data_add.cpp
    recv_sys_create.cpp
    recv_sys_resize_buf.cpp
    recv_sys_finish.cpp
    recv_sys_close.cpp
    recv_sys_var_init.cpp
    recv_heap_used.cpp
    recv_report_corrupt_log.cpp
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
    buf_page
    log_write
    PRIVATE
)

