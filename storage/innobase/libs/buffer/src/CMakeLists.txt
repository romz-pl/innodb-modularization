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

