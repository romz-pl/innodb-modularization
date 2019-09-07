set( LIB_NAME trx_rseq )

add_library( ${LIB_NAME} STATIC
    trx_rsegf_get.cpp
    trx_rsegf_get_new.cpp
    trx_rsegf_get_nth_undo.cpp
    trx_rsegf_set_nth_undo.cpp
    trx_rsegf_undo_find_free.cpp
    trx_rseg_header_create.cpp
    trx_rseg_adjust_rollback_segments.cpp
    trx_rseg_init_rollback_segments.cpp
    trx_rsegs_init.cpp
    trx_rseg_mem_create.cpp
    trx_rseg_create.cpp
    trx_rseg_get_n_undo_tablespaces.cpp
    trx_rseg_upgrade_undo_tablespaces.cpp
    trx_rseg_array_create.cpp
    trx_rsegsf_set_page_no.cpp
    flags.cpp
    trx_rseg_id_to_space_id.cpp
    trx_rsegsf_get.cpp
    trx_rsegsf_get_page_no.cpp
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
    trx_types
    PRIVATE
)

