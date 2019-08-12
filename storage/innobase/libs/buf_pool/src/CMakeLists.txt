set( LIB_NAME buf_pool )

add_library( ${LIB_NAME} STATIC
    BufListMutex.cpp
    BufPoolZipMutex.cpp
    Cache_hint.cpp
    CheckInFreeList.cpp
    CheckInLRUList.cpp
    CheckUnzipLRUAndLRUList.cpp
    FlushHp.cpp
    FlushListMutex.cpp
    HazardPointer.cpp
    LRUHp.cpp
    LRUItr.cpp
    Page_fetch.cpp
    buf_get_total_list_len.cpp
    buf_get_total_list_size_in_bytes.cpp
    buf_get_total_stat.cpp
    buf_get_withdraw_depth.cpp
    buf_pool_from_array.cpp
    buf_pool_from_block.cpp
    buf_pool_from_bpage.cpp
    buf_pool_get.cpp
    buf_pool_get_curr_size.cpp
    buf_pool_get_n_pages.cpp
    buf_pool_get_oldest_modification_approx.cpp
    buf_pool_index.cpp
    buf_pool_info_t.cpp
    buf_pool_is_obsolete.cpp
    buf_pool_ptr.cpp
    buf_pool_register_chunk.cpp
    buf_pool_set_sizes.cpp
    buf_pool_should_madvise.cpp
    buf_pool_size_align.cpp
    buf_pool_stat_t.cpp
    buf_pool_t.cpp
    buf_pool_update_madvise.cpp
    buf_pool_withdrawing.cpp
    buf_pools_list_size_t.cpp
    buf_ptr_get_fsp_addr.cpp
    buf_withdraw_clock.cpp
    innobase_disable_core_dump.cpp
    innobase_should_madvise_buf_pool.cpp
    srv_buf_pool_base_size.cpp
    srv_buf_pool_chunk_unit.cpp
    srv_buf_pool_curr_size.cpp
    srv_buf_pool_def_size.cpp
    srv_buf_pool_dump_pct.cpp
    srv_buf_pool_instances.cpp
    srv_buf_pool_instances_default.cpp
    srv_buf_pool_min_size.cpp
    srv_buf_pool_old_size.cpp
    srv_buf_pool_size.cpp
    srv_buffer_pool_in_core_file.cpp
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
    buf_buddy
    buf_block
    buf_chunk
    buf_flush
    tablespace
    PRIVATE
)
