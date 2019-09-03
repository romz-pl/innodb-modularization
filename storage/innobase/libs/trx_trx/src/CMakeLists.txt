set( LIB_NAME trx_trx )

add_library( ${LIB_NAME} STATIC
    trx_undo_ptr_t.cpp
    trx_rsegs_t.cpp
    trx_t.cpp
    lock_pool_t.cpp
    trx_lock_t.cpp
    TrxVersion.cpp
    hit_list_t.cpp
    trx_mod_tables_t.cpp
    trx_is_autocommit_non_locking.cpp
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
    lock_types
    log_sn
    memory
    trx_types
    vector
    PRIVATE
)

