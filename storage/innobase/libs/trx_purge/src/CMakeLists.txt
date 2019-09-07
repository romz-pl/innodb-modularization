set( LIB_NAME trx_purge )

add_library( ${LIB_NAME} STATIC
    trx_purge_sys_close.cpp
    trx_purge_sys_create.cpp
    trx_purge_add_update_undo_to_history.cpp
    trx_purge.cpp
    trx_purge_stop.cpp
    trx_purge_run.cpp
    purge_state_t.cpp
    trx_purge_state.cpp
    purge_iter_t.cpp
    flags.cpp
    space_id_account.cpp
    space_id_bank.cpp
    is_reserved.cpp
    num2id.cpp
    id2num.cpp
    id2next_id.cpp
    init_space_id_bank.cpp
    use_space_id.cpp
    use_next_space_id.cpp
    unuse_space_id.cpp
    next_space_id.cpp
    get_next_available_space_num.cpp
    make_space_name.cpp
    make_file_name.cpp
    Tablespace.cpp
    Tablespaces.cpp
    ddl_mutex.cpp
    spaces.cpp
    start_logging.cpp
    done_logging.cpp
    is_active_truncate_log_present.cpp
    s_under_construction.cpp
    add_space_to_construction_list.cpp
    clear_construction_list.cpp
    is_under_construction.cpp
    set_active.cpp
    is_active.cpp
    Truncate.cpp
    srv_purge_rseg_truncate_frequency.cpp
    trx_purge_t.cpp
    TrxUndoRsegsIterator.cpp
    trx_purge_get_log_from_hist.cpp
    purge_sys.cpp
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
    read
    tablespace
    trx_types
    PRIVATE
)

