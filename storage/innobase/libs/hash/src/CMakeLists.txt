set( LIB_NAME hash )

add_library( ${LIB_NAME} STATIC
    hash_node_t.cpp
    hash_table_sync_t.cpp
    hash_create.cpp
    hash_cell_t.cpp
    hash_table_t.cpp
    hash_get_nth_cell.cpp
    hash_table_clear.cpp
    hash_get_n_cells.cpp
    hash_calc_hash.cpp
    hash_get_sync_obj_index.cpp
    hash_get_nth_heap.cpp
    hash_get_heap.cpp
    hash_get_nth_mutex.cpp
    hash_get_nth_lock.cpp
    hash_get_mutex.cpp
    hash_get_lock.cpp
    hash_lock_s_confirm.cpp
    hash_lock_x_confirm.cpp
    hash_lock_x_all.cpp
    hash_unlock_x_all.cpp
    hash_unlock_x_all_but.cpp
    hash_table_free.cpp
    hash_create_sync_obj.cpp
    HASH_ASSERT_OWN.cpp
    HASH_INSERT.cpp
    HASH_ASSERT_VALID.cpp
    HASH_INVALIDATE.cpp
    HASH_DELETE.cpp
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
    random
    sync_rw
    PRIVATE
)

