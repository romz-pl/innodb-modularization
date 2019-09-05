set( LIB_NAME trx_trx )

add_library( ${LIB_NAME} STATIC
    TrxVersion.cpp
    assert_trx_in_rw_list.cpp
    check_trx_state.cpp
    hit_list_t.cpp
    lock_pool_t.cpp
    thd_trx_priority.cpp
    trx_get_dict_operation.cpp
    trx_get_error_index.cpp
    trx_get_id_for_print.cpp
    trx_get_que_state_str.cpp
    trx_is_ac_nl_ro.cpp
    trx_is_autocommit_non_locking.cpp
    trx_is_high_priority.cpp
    trx_is_redo_rseg_updated.cpp
    trx_is_referenced.cpp
    trx_is_rseg_assigned.cpp
    trx_is_rseg_updated.cpp
    trx_is_temp_rseg_updated.cpp
    trx_lock_t.cpp
    trx_mod_tables_t.cpp
    trx_mutex_enter.cpp
    trx_mutex_exit.cpp
    trx_mutex_own.cpp
    trx_reference.cpp
    trx_release_reference.cpp
    trx_rsegs_t.cpp
    trx_set_dict_operation.cpp
    trx_state_eq.cpp
    trx_t.cpp
    trx_undo_ptr_t.cpp
    thd_trx_arbitrate.cpp
    trx_arbitrate.cpp
    trx_set_flush_observer.cpp
    flags.cpp
    trx_set_detailed_error.cpp
    trx_set_detailed_error_from_file.cpp
    table_id_set.cpp
    trx_table_map.cpp
    resurrected_trx_tables.cpp
    trx_dummy_sess.cpp
    trx_erase_lists.cpp
    TRX_WEIGHT.cpp
    trx_lock_wait_timeout_get.cpp
    assert_trx_is_free.cpp
    assert_trx_is_inactive.cpp
    assert_trx_nonlocking_or_in_list.cpp
    trx_immutable_id.cpp
    trx_is_started.cpp
    trx_get_read_view.cpp
    trx_isolation_level.cpp
    TrxInInnoDB.cpp
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
    io
    lock_types
    log_sn
    read
    string
    trx_sys
    vector
    PRIVATE
)

