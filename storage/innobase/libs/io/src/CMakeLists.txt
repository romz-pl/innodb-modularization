set( LIB_NAME io )

add_library( ${LIB_NAME} STATIC
    Block.cpp
    Blocks.cpp
    Compression.cpp
    Dir_Walker.cpp
    Encryption.cpp
    Fil_path.cpp
    IORequest.cpp
    SyncFileIO.cpp
    access_type.cpp
    block_cache.cpp
    ib_file_suffix.cpp
    meb_free_block_cache.cpp
    os_alloc_block.cpp
    os_create_block_cache.cpp
    os_file_can_delete.cpp
    os_file_close_func.cpp
    os_file_compress_page.cpp
    os_file_compressed_page_size.cpp
    os_file_create_directory.cpp
    os_file_create_func.cpp
    os_file_create_simple_func.cpp
    os_file_create_simple_no_error_handling_func.cpp
    os_file_create_subdirs_if_needed.cpp
    os_file_create_t.cpp
    os_file_delete_func.cpp
    os_file_delete_if_exists_func.cpp
    os_file_encrypt_log.cpp
    os_file_encrypt_page.cpp
    os_file_flush_func.cpp
    os_file_fsync_posix.cpp
    os_file_get_last_error.cpp
    os_file_get_last_error_low.cpp
    os_file_get_parent_dir.cpp
    os_file_get_size.cpp
    os_file_get_status.cpp
    os_file_get_status_posix.cpp
    os_file_handle_error.cpp
    os_file_handle_error_cond_exit.cpp
    os_file_handle_error_no_exit.cpp
    os_file_io.cpp
    os_file_io_complete.cpp
    os_file_is_root.cpp
    os_file_lock.cpp
    os_file_original_page_size.cpp
    os_file_punch_hole.cpp
    os_file_punch_hole_posix.cpp
    os_file_rename_func.cpp
    os_file_scan_directory.cpp
    os_file_seek.cpp
    os_file_set_eof.cpp
    os_file_set_nocache.cpp
    os_file_set_umask.cpp
    os_file_size_t.cpp
    os_file_stat_t.cpp
    os_file_status.cpp
    os_file_status_posix.cpp
    os_file_t.cpp
    os_file_truncate.cpp
    os_file_truncate_posix.cpp
    os_file_type_t.cpp
    os_free_block.cpp
    os_innodb_umask.cpp
    os_is_o_direct_supported.cpp
    os_is_sparse_file_supported.cpp
    os_offset_t.cpp
    pfs_os_file_t.cpp
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
    page
    PRIVATE
)

