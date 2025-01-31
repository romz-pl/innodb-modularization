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
    trx_init.cpp
    TrxFactory.cpp
    trx_named_savept_t.cpp
    TrxPoolLock.cpp
    TrxPoolManagerLock.cpp
    trx_pool_t.cpp
    Pool.cpp
    PoolManager.cpp
    trx_pools_t.cpp
    trx_pools.cpp
    trx_pool_init.cpp
    trx_pool_close.cpp
    trx_create_low.cpp
    trx_free.cpp
    trx_allocate_for_background.cpp
    trx_allocate_for_mysql.cpp
    trx_validate_state_before_free.cpp
    trx_free_resurrected.cpp
    trx_free_for_background.cpp
    trx_release_impl_and_expl_locks.cpp
    trx_free_prepared.cpp
    trx_disconnect_from_mysql.cpp
    trx_disconnect_plain.cpp
    trx_disconnect_prepared.cpp
    trx_free_for_mysql.cpp
    trx_resurrect_locks.cpp
    trx_lists_init_at_db_start.cpp
    trx_start_if_not_started_xa_low.cpp
    trx_start_if_not_started_low.cpp
    trx_start_internal_low.cpp
    trx_start_internal_read_only_low.cpp
    trx_start_if_not_started_xa.cpp
    trx_start_if_not_started.cpp
    trx_start_internal.cpp
    trx_start_internal_read_only.cpp
    trx_commit.cpp
    trx_commit_low.cpp
    trx_cleanup_at_db_startup.cpp
    trx_commit_for_mysql.cpp
    trx_prepare_for_mysql.cpp
    trx_recover_for_mysql.cpp
    trx_get_trx_by_xid.cpp
    trx_commit_complete_for_mysql.cpp
    trx_mark_sql_stat_end.cpp
    trx_assign_read_view.cpp
    trx_commit_or_rollback_prepare.cpp
    trx_commit_node_create.cpp
    trx_commit_step.cpp
    trx_print_low.cpp
    trx_print_latched.cpp
    trx_print.cpp
    trx_assert_started.cpp
    trx_is_interrupted.cpp
    trx_is_strict.cpp
    trx_weight_ge.cpp
    trx_assign_rseg_temp.cpp
    trx_set_rw_mode.cpp
    trx_kill_blocking.cpp
    trx_rseg_type_t.cpp
    commit_node_state.cpp
    commit_node_t.cpp
    trx_resurrect_table_ids.cpp
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
    mtr
    que_types
    read
    string
    trx_rec
    trx_undo
    trx_sys
    vector
    PRIVATE
)

