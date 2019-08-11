set( LIB_NAME buffer )

add_library( ${LIB_NAME} STATIC
    BPageLock.cpp
    BPageMutex.cpp
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
    buf_buddy_free_t.cpp
    buf_buddy_stat_t.cpp
    buf_chunk_init.cpp
    buf_chunk_map_reg.cpp
    buf_chunk_not_freed.cpp
    buf_chunk_t.cpp
    buf_flush_list_mutex_enter.cpp
    buf_flush_list_mutex_exit.cpp
    buf_flush_list_mutex_own.cpp
    buf_flush_ready_for_replace.cpp

    buf_frame_align.cpp
    buf_frame_copy.cpp
    buf_frame_t.cpp
    buf_get_nth_chunk_block.cpp
    buf_get_total_list_len.cpp
    buf_get_total_list_size_in_bytes.cpp
    buf_get_total_stat.cpp
    buf_get_withdraw_depth.cpp


    buf_page_belongs_to_unzip_LRU.cpp
    buf_page_can_relocate.cpp

    buf_page_get_block.cpp


    buf_page_get_io_fix.cpp
    buf_page_get_io_fix_unlocked.cpp
    buf_page_get_mutex.cpp
    buf_page_get_newest_modification.cpp






    buf_page_release_latch.cpp
    buf_page_release_zip.cpp
    buf_page_set_accessed.cpp

    buf_page_set_io_fix.cpp
    buf_page_set_old.cpp

    buf_page_set_sticky.cpp


    buf_page_unset_sticky.cpp
    buf_pool_chunk_map_t.cpp
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
    srv_checksum_algorithm_t.cpp
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
    tablespace
    PRIVATE
)

