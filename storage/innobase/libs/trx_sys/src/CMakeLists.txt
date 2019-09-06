set( LIB_NAME trx_sys )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    trx_sys_t.cpp
    trx_ut_list_t.cpp
    trx_sys.cpp
    trx_sys_mutex_own.cpp
    trx_sys_mutex_enter.cpp
    trx_sys_mutex_exit.cpp
    trx_sys_get_max_trx_id.cpp
    trx_sys_hdr_page.cpp
    trx_sys_init_at_db_start.cpp
    trx_sys_create.cpp
    trx_sysf_rseg_t.cpp
    trx_sys_flush_max_trx_id.cpp
    trx_sys_create_sys_pages.cpp
    trx_sysf_rseg_find_page_no.cpp
    trx_sysf_rseg_find_free.cpp
    trx_sysf_get.cpp
    trx_sysf_rseg_get_space.cpp
    trx_sysf_rseg_get_page_no.cpp
    trx_sysf_rseg_set_space.cpp
    trx_sysf_rseg_set_page_no.cpp
    trx_sys_get_new_trx_id.cpp
    trx_rseg_n_slots_debug.cpp
    trx_write_trx_id.cpp
    trx_read_trx_id.cpp
    trx_get_rw_trx_by_id.cpp
    trx_rw_min_trx_id.cpp
    trx_rw_is_active_low.cpp
    trx_rw_is_active.cpp
    trx_assert_recovered.cpp
    trx_sys_update_mysql_binlog_offset.cpp
    trx_sys_print_mysql_binlog_offset.cpp
    trx_sys_close.cpp
    trx_sys_need_rollback.cpp
    trx_sys_any_active_transactions.cpp
    trx_sys_print_mysql_binlog_offset_from_page.cpp
    trx_sys_rw_trx_add.cpp
    trx_sys_validate_trx_list.cpp
    trx_sys_undo_spaces_init.cpp
    trx_sys_undo_spaces_deinit.cpp
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
    data_types
    trx_types
    PRIVATE
)

