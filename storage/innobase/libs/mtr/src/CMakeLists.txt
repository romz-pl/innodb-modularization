set( LIB_NAME mtr )

add_library( ${LIB_NAME} STATIC
    dyn_buf_t.cpp
    mtr_log_t.cpp
    mtr_memo_type_t.cpp
    mtr_state_t.cpp
    MTR_MAGIC_N.cpp
    mtr_buf_t.cpp
    mtr_buf_copy_t.cpp
    mtr_memo_slot_t.cpp
    mtr_t.cpp
    mtr_start.cpp
    mtr_start_sync.cpp
    mtr_start_ro.cpp
    mtr_commit.cpp
    mtr_set_savepoint.cpp
    mtr_release_s_latch_at_savepoint.cpp
    mtr_get_log_mode.cpp
    mtr_set_log_mode.cpp
    mtr_get_flush_observer.cpp
    mtr_set_flush_observer.cpp
    mtr_read_ulint.cpp
    mtr_memo_release.cpp
    mtr_is_block_fix.cpp
    mtr_is_page_fix.cpp
    mtr_memo_contains.cpp
    mtr_memo_contains_page.cpp
    mtr_print.cpp
    mtr_get_log.cpp
    mtr_memo_push.cpp
    mtr_s_lock.cpp
    mtr_x_lock.cpp
    mtr_x_lock_space.cpp
    mtr_sx_lock.cpp
    mtr_memo_contains_flagged.cpp
    mtr_memo_contains_page_flagged.cpp
    mtr_release_block_at_savepoint.cpp
    mtr_block_sx_latch_at_savepoint.cpp
    mtr_block_x_latch_at_savepoint.cpp
    mtr_block_dirtied.cpp
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
    buffer
    memory
    sync_rw
    PRIVATE
)

