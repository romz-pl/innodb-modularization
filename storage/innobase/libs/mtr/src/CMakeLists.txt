set( LIB_NAME mtr )

add_library( ${LIB_NAME} STATIC
    Add_dirty_blocks_to_flush_list.cpp
    Command.cpp
    Debug_check.cpp
    Find.cpp
    Find_page.cpp
    Iterate.cpp
    MTR_MAGIC_N.cpp
    Release_all.cpp
    dyn_buf_t.cpp
    memo_slot_release.cpp
    mlog_catenate_string.cpp
    mlog_catenate_ulint.cpp
    mlog_catenate_ulint_compressed.cpp
    mlog_catenate_ull_compressed.cpp
    mlog_close.cpp
    mlog_log_string.cpp
    mlog_open.cpp
    mlog_open_and_write_index.cpp
    mlog_open_metadata.cpp
    mlog_parse_initial_dict_log_record.cpp
    mlog_parse_initial_log_record.cpp
    mlog_write_initial_dict_log_record.cpp
    mlog_write_initial_log_record.cpp
    mlog_write_initial_log_record_fast.cpp
    mlog_write_initial_log_record_low.cpp
    mlog_write_string.cpp
    mlog_write_ulint.cpp
    mlog_write_ull.cpp
    mtr_block_dirtied.cpp
    mtr_block_sx_latch_at_savepoint.cpp
    mtr_block_x_latch_at_savepoint.cpp
    mtr_buf_copy_t.cpp
    mtr_buf_t.cpp
    mtr_commit.cpp
    mtr_get_flush_observer.cpp
    mtr_get_log.cpp
    mtr_get_log_mode.cpp
    mtr_is_block_fix.cpp
    mtr_is_page_fix.cpp
    mtr_log_t.cpp
    mtr_memo_contains.cpp
    mtr_memo_contains_flagged.cpp
    mtr_memo_contains_page.cpp
    mtr_memo_contains_page_flagged.cpp
    mtr_memo_push.cpp
    mtr_memo_release.cpp
    mtr_memo_slot_t.cpp
    mtr_memo_type_t.cpp
    mtr_print.cpp
    mtr_read_ulint.cpp
    mtr_release_block_at_savepoint.cpp
    mtr_release_s_latch_at_savepoint.cpp
    mtr_s_lock.cpp
    mtr_set_flush_observer.cpp
    mtr_set_log_mode.cpp
    mtr_set_savepoint.cpp
    mtr_start.cpp
    mtr_start_ro.cpp
    mtr_start_sync.cpp
    mtr_state_t.cpp
    mtr_sx_lock.cpp
    mtr_t.cpp
    mtr_x_lock.cpp
    mtr_x_lock_space.cpp
    mtr_write_log_t.cpp
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
    log_write
    PRIVATE
)

