set( LIB_NAME lock_priv )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    lock_cancel_waiting_and_release.cpp
    lock_clust_rec_some_has_impl.cpp
    lock_compatibility_matrix.cpp
    lock_get_mode.cpp
    lock_get_type_low.cpp
    lock_get_wait.cpp
    lock_mode_compatible.cpp
    lock_mode_stronger_or_eq.cpp
    lock_rec_find_similar_on_page.cpp
    lock_rec_fold.cpp
    lock_rec_get_first.cpp
    lock_rec_get_first_on_page.cpp
    lock_rec_get_first_on_page_addr.cpp
    lock_rec_get_n_bits.cpp
    lock_rec_get_next.cpp
    lock_rec_get_next_const.cpp
    lock_rec_get_next_on_page.cpp
    lock_rec_get_next_on_page_const.cpp
    lock_rec_get_nth_bit.cpp
    lock_rec_get_prev.cpp
    lock_rec_req_status.cpp
    lock_rec_set_nth_bit.cpp
    lock_rec_t.cpp
    lock_reset_wait_and_release_thread_if_suspended.cpp
    lock_strength_matrix.cpp
    lock_t.cpp
    lock_table_has.cpp
    lock_table_t.cpp
    lock_rec_hash.cpp
    lock_hash_get.cpp
    lock_rec_find_set_bit.cpp
    lock_rec_find_next_set_bit.cpp
    lock_get_size.cpp
    lock_move_reorganize_page.cpp
    lock_move_rec_list_end.cpp
    lock_move_rec_list_start.cpp
    lock_update_split_right.cpp
    lock_update_merge_right.cpp
    lock_update_root_raise.cpp
    lock_update_copy_and_discard.cpp
    lock_update_split_left.cpp
    lock_update_merge_left.cpp
    lock_rec_reset_and_inherit_gap_locks.cpp
    lock_update_discard.cpp
    lock_update_insert.cpp
    lock_update_delete.cpp
    lock_rec_store_on_page_infimum.cpp
    lock_rec_restore_from_page_infimum.cpp
    lock_rec_expl_exist_on_page.cpp
    lock_rec_reset_nth_bit.cpp
    lock_reset_lock_and_trx_wait.cpp
    lock_rec_trx_wait.cpp
    lock_rec_bitmap_reset.cpp
    lock_rec_copy.cpp
    lock_rec_get_prev.cpp
    lock_rec_get_insert_intention.cpp
    lock_rec_get_rec_not_gap.cpp
    lock_rec_get_gap.cpp
    lock_rec_lock_fold.cpp
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
    buf_block
    dict_mem
    hash
    lock_sys
    lock_types
    trx_types
    PRIVATE
)

