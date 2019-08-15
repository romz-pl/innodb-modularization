set( LIB_NAME dict_mem )

add_library( ${LIB_NAME} STATIC
    dict_add_v_col_t.cpp
    dict_col_default_t.cpp
    dict_col_t.cpp
    dict_field_t.cpp
    dict_foreign_compare.cpp
    dict_foreign_different_tables.cpp
    dict_foreign_matches_id.cpp
    dict_foreign_print.cpp
    dict_foreign_set.cpp
    dict_foreign_t.cpp
    dict_foreign_with_index.cpp
    dict_index_t.cpp
    dict_s_col_list.cpp
    dict_s_col_t.cpp
    dict_v_col_t.cpp
    dict_v_idx_list.cpp
    dict_v_idx_t.cpp
    dict_vcol_set.cpp
    flags.cpp
    last_ops_cur_t.cpp
    online_index_status.cpp
    os_once.cpp
    rec_cache_t.cpp
    zip_pad_info_t.cpp
    dict_foreign_not_exists.cpp
    dict_foreign_set_free.cpp
    dict_foreign_free.cpp
    dict_foreign_set_validate.cpp
    dict_vcol_templ_t.cpp
    table_dirty_status.cpp
    temp_prebuilt_vec.cpp
    table_lock_list_t.cpp
    dict_table_t.cpp
    persistent_type_t.cpp
    corrupted_ids_t.cpp
    PersistentTableMetadata.cpp
    Persister.cpp
    CorruptedIndexPersister.cpp
    AutoIncPersister.cpp
    Persisters.cpp
    lock_table_lock_list_init.cpp
    dict_foreign_add_to_referenced_table.cpp
    dict_table_mutex_create_lazy.cpp
    dict_table_mutex_destroy.cpp
    dict_table_autoinc_destroy.cpp
    dict_table_autoinc_create_lazy.cpp
    dict_index_zip_pad_mutex_create_lazy.cpp
    dict_index_zip_pad_mutex_destroy.cpp
    dict_index_zip_pad_unlock.cpp
    dict_table_autoinc_own.cpp
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
    data_types
    dict_types
    gis_type
    lock_priv
    string
    PRIVATE
)

