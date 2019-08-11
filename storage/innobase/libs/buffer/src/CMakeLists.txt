set( LIB_NAME buffer )

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
    buf_flush_list_mutex_enter.cpp
    buf_flush_list_mutex_exit.cpp
    buf_flush_list_mutex_own.cpp
    buf_flush_ready_for_replace.cpp
    buf_get_total_list_len.cpp
    buf_get_total_list_size_in_bytes.cpp
    buf_get_total_stat.cpp
    buf_get_withdraw_depth.cpp
    buf_pools_list_size_t.cpp
    buf_ptr_get_fsp_addr.cpp
    buf_withdraw_clock.cpp
    innobase_disable_core_dump.cpp
    innobase_should_madvise_buf_pool.cpp
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
    buf_chunk
    buf_pool
    tablespace
    PRIVATE
)

