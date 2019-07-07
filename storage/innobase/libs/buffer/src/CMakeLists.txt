set( LIB_NAME buffer )

add_library( ${LIB_NAME} STATIC
    BPageLock.cpp
    BPageMutex.cpp
    BufListMutex.cpp
    BufPoolZipMutex.cpp
    Cache_hint.cpp
    FlushListMutex.cpp
    Page_fetch.cpp
    buf_flush_t.cpp
    buf_frame_t.cpp
    buf_io_fix.cpp
    buf_page_print_flags.cpp
    buf_page_state.cpp
    buf_pool_info_t.cpp
    buf_pools_list_size_t.cpp
    macros.cpp
    page_id_t.cpp
    srv_checksum_algorithm_t.cpp
    lsn_t.cpp
    buf_page_t.cpp
    buf_block_t.cpp
    HazardPointer.cpp
    FlushHp.cpp
    LRUHp.cpp
    LRUItr.cpp
    buf_buddy_free_t.cpp
    buf_pool_stat_t.cpp
    buf_buddy_stat_t.cpp
    buf_pool_t.cpp
    buf_flush_list_mutex_own.cpp
    buf_flush_list_mutex_enter.cpp
    buf_flush_list_mutex_exit.cpp
    buf_page_mutex_own.cpp
    buf_page_mutex_enter.cpp
    buf_page_mutex_exit.cpp
    buf_pool_ptr.cpp
    buf_pool_withdrawing.cpp
    buf_withdraw_clock.cpp
    buf_pool_chunk_map_t.cpp
    buf_chunk_map_reg.cpp
    buf_chunk_t.cpp
    buf_pool_get_curr_size.cpp
    buf_pool_index.cpp
    buf_pool_from_bpage.cpp
    buf_pool_from_block.cpp
    buf_pool_get_n_pages.cpp
    buf_page_get_freed_page_clock.cpp
    buf_block_get_freed_page_clock.cpp
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
    tablespace
    PRIVATE
)

