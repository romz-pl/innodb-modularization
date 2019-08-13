set( LIB_NAME dict_mem )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    dict_col_default_t.cpp
    dict_col_t.cpp
    dict_v_idx_t.cpp
    dict_v_idx_list.cpp
    dict_v_col_t.cpp
    dict_add_v_col_t.cpp
    dict_s_col_t.cpp
    dict_s_col_list.cpp
    dict_field_t.cpp
    os_once.cpp
    zip_pad_info_t.cpp
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
    PRIVATE
)

