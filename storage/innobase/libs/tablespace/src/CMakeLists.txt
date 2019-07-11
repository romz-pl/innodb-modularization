set( LIB_NAME tablespace )

add_library( ${LIB_NAME} STATIC
    AIO_mode.cpp
    Char_Ptr_Compare.cpp
    Char_Ptr_Hash.cpp
    Datafile.cpp
    Dirs.cpp
    Fil_iterator.cpp
    Fil_shard.cpp
    Fil_state.cpp
    Fil_system.cpp
    Scanned_files.cpp
    Space_id_set.cpp
    Tablespace_dirs.cpp
    Tablespace_files.cpp
    Tablespaces.cpp
    buf_remove_t.cpp
    consts.cpp
    device_t.cpp
    encryption_op_type.cpp
    extent.cpp
    fil_addr_t.cpp
    fil_faddr_t.cpp
    fil_load_status.cpp
    fil_n_file_opened.cpp
    fil_no_punch_hole.cpp
    fil_node_t.cpp
    fil_operation_t.cpp
    fil_space_t.cpp
    fil_type_t.cpp
    fsp_header_get_field.cpp
    fsp_header_get_flags.cpp
    fsp_header_get_page_size.cpp
    fsp_header_get_space_id.cpp
    header.cpp
    lsn_t.cpp
    os_file_read_first_page.cpp
    os_file_read_first_page_func.cpp
    os_file_read_first_page_pfs.cpp
    page_id_t.cpp
    pfs_os_file_read_first_page_func.cpp
    space_id_t.cpp
    srv_start_raw_disk_in_use.cpp
    fil_buffering_disabled.cpp
    FLST_LEN.cpp
    flst_get_len.cpp
    recv_recovery_on.cpp
    Tablespace.cpp
    fil_fusionio_enable_atomic_write.cpp
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
    io
    memory
    string
    sync_rw
    PRIVATE
)

