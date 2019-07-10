set( LIB_NAME tablespace )

add_library( ${LIB_NAME} STATIC
    Char_Ptr_Compare.cpp
    Char_Ptr_Hash.cpp
    Datafile.cpp
    Dirs.cpp
    Fil_state.cpp
    Scanned_files.cpp
    Space_id_set.cpp
    Tablespace_dirs.cpp
    Tablespace_files.cpp
    Tablespaces.cpp
    buf_remove_t.cpp
    device_t.cpp
    encryption_op_type.cpp
    fil_addr_t.cpp
    fil_faddr_t.cpp
    fil_load_status.cpp
    fil_no_punch_hole.cpp
    fil_node_t.cpp
    fil_operation_t.cpp
    fil_space_t.cpp
    fil_type_t.cpp
    lsn_t.cpp
    space_id_t.cpp
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
    string
    sync_rw
    PRIVATE
)

