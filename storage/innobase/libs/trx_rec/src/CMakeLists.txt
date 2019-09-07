set( LIB_NAME trx_rec )

add_library( ${LIB_NAME} STATIC
    trx_undo_rec_copy.cpp
    trx_undo_rec_get_type.cpp
    trx_undo_rec_get_cmpl_info.cpp
    trx_undo_rec_get_extern_storage.cpp
    trx_undo_rec_get_undo_no.cpp
    trx_undo_rec_get_ptr.cpp
    trx_undo_rec_get_table_id.cpp
    trx_undo_rec_get_row_ref.cpp
    trx_undo_update_rec_get_sys_cols.cpp
    trx_undo_update_rec_get_update.cpp
    trx_undo_rec_get_partial_row.cpp
    trx_undo_report_row_operation.cpp
    flags.cpp
    trx_undo_prev_version_build.cpp
    trx_undo_parse_add_undo_rec.cpp
    trx_undo_parse_erase_page_end.cpp
    trx_undo_rec_get_col_val.cpp
    trx_undo_read_v_cols.cpp
    trx_undo_read_v_idx.cpp
    type_cmpl_t.cpp
    trx_undo_rec_get_pars.cpp
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
    memory
    trx_types
    PRIVATE
)

