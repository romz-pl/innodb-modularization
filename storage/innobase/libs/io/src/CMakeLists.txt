set( LIB_NAME io )

add_library( ${LIB_NAME} STATIC
    access_type.cpp
    AIO_mode.cpp
    Block.cpp
    block_cache.cpp
    Blocks.cpp
    Compression.cpp
    Encryption.cpp
    IORequest.cpp
    pfs_os_file_t.cpp
    os_alloc_block.cpp
    os_free_block.cpp
    os_file_compress_page.cpp
    meb_free_block_cache.cpp
    os_create_block_cache.cpp
    os_file_close_func.cpp
    os_file_create_directory.cpp
    os_file_create_func.cpp
    os_file_create_subdirs_if_needed.cpp
    os_file_create_simple_func.cpp
    os_file_create_simple_no_error_handling_func.cpp
    os_file_create_t.cpp
    os_file_delete_func.cpp
    os_file_get_last_error.cpp
    os_file_get_size.cpp
    os_file_get_status_posix.cpp
    os_file_get_last_error_low.cpp
    os_file_get_parent_dir.cpp
    os_file_handle_error.cpp
    os_file_handle_error_cond_exit.cpp
    os_file_handle_error_no_exit.cpp
    os_file_is_root.cpp
    os_file_lock.cpp
    os_file_rename_func.cpp
    os_file_scan_directory.cpp
    os_file_set_nocache.cpp
    os_file_set_umask.cpp
    os_file_size_t.cpp
    os_file_stat_t.cpp
    os_file_status.cpp
    os_file_status_posix.cpp
    os_file_t.cpp
    os_file_type_t.cpp
    os_innodb_umask.cpp
    os_offset_t.cpp
    srv_unix_flush_t.cpp
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
    allocator
    error
    logger
    page
    thread
    PRIVATE
)

