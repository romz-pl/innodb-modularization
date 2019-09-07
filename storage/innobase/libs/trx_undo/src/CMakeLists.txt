set( LIB_NAME trx_undo )

add_library( ${LIB_NAME} STATIC
    trx_undo_roll_ptr_is_insert.cpp
    trx_undo_trx_id_is_insert.cpp
    trx_write_roll_ptr.cpp
    trx_read_roll_ptr.cpp
    trx_undo_page_get.cpp
    trx_undo_page_get_s_latched.cpp
    trx_undo_page_get_prev_rec.cpp
    trx_undo_page_get_next_rec.cpp
    trx_undo_page_get_last_rec.cpp
    trx_undo_get_prev_rec.cpp
    trx_undo_get_next_rec.cpp
    trx_undo_get_first_rec.cpp
    trx_undo_add_page.cpp
    trx_undo_free_last_page_func.cpp
    trx_undo_free_last_page.cpp
    trx_undo_truncate_end_func.cpp
    trx_undo_truncate_end.cpp
    trx_undo_truncate_start.cpp
    trx_undo_lists_init.cpp
    trx_undo_assign_undo.cpp
    trx_undo_set_state_at_finish.cpp
    trx_undo_set_state_at_prepare.cpp
    trx_undo_update_cleanup.cpp
    trx_undo_insert_cleanup.cpp
    trx_undo_free_prepared.cpp
    trx_undo_truncate_tablespace.cpp
    trx_undo_parse_page_init.cpp
    trx_undo_parse_page_header.cpp
    trx_undo_mem_free.cpp
    flags.cpp
    trx_undo_t.cpp
    trx_undo_build_roll_ptr.cpp
    trx_undo_decode_roll_ptr.cpp
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
    log_types
    trx_types
    PRIVATE
)

