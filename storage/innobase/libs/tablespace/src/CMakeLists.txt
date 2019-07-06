set( LIB_NAME tablespace )

add_library( ${LIB_NAME} STATIC
    buf_remove_t.cpp
    Char_Ptr_Compare.cpp
    Char_Ptr_Hash.cpp
    Dirs.cpp
    encryption_op_type.cpp
    fil_addr_t.cpp
    fil_faddr_t.cpp
    fil_load_status.cpp
    fil_node_t.cpp
    fil_operation_t.cpp
    fil_type_t.cpp
    fil_space_t.cpp
    Fil_state.cpp
    Scanned_files.cpp
    Space_id_set.cpp
    space_id_t.cpp
    Tablespace_dirs.cpp
    Tablespace_files.cpp
    Tablespaces.cpp
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
    sync_rw
    PRIVATE
)

