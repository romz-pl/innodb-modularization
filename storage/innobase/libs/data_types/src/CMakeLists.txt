set( LIB_NAME data_types )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    dtype_t.cpp
    data_mysql_default_charset_coll.cpp
    ib_like_t.cpp
    dtype_get_mysql_type.cpp
    dtype_is_string_type.cpp
    dtype_is_binary_string_type.cpp
    dtype_is_non_binary_string_type.cpp
    dtype_set.cpp
    dtype_get_charset_coll.cpp
    dtype_is_utf8.cpp
    dtype_get_mblen.cpp
    innobase_get_cset_width.cpp
    dtype_set_mbminmaxlen.cpp
    dtype_set_mblen.cpp
    dtype_get_mbminlen.cpp
    dtype_get_mbmaxlen.cpp
    dtype_validate.cpp
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
    assert
    PRIVATE
)

