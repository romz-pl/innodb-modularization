set( LIB_NAME buf_block )

add_library( ${LIB_NAME} STATIC
    BPageLock.cpp
    BPageMutex.cpp
    buf_block_buf_fix_dec.cpp
    buf_block_buf_fix_inc.cpp
    buf_block_buf_fix_inc_func.cpp
    buf_block_fix.cpp
    buf_block_get_frame.cpp
    buf_block_get_freed_page_clock.cpp
    buf_block_get_io_fix.cpp
    buf_block_get_io_fix_unlocked.cpp
    buf_block_get_lock_hash_val.cpp
    buf_block_get_state.cpp
    buf_block_init.cpp
    buf_block_set_file_page.cpp
    buf_block_set_io_fix.cpp
    buf_block_set_state.cpp
    buf_block_state_valid.cpp
    buf_block_t.cpp
    buf_block_unfix.cpp
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
    buf_frame
    buf_page
    buf_pool
    sync_rw
    PRIVATE
)

