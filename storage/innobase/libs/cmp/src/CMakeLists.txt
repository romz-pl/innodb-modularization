set( LIB_NAME cmp )

add_library( ${LIB_NAME} STATIC
    cmp_cols_are_equal.cpp
    cmp_decimal.cpp
    innobase_mysql_cmp.cpp
    cmp_geometry_field.cpp
    cmp_whole_field.cpp
    cmp_data.cpp
    cmp_data_data.cpp
    cmp_dfield_dfield.cpp
    cmp_dtuple_rec_with_gis.cpp
    cmp_gis_field.cpp
    cmp_dtuple_rec_with_gis_internal.cpp
    cmp_dtuple_rec_with_match_low.cpp
    cmp_dtuple_rec_with_match.cpp
    cmp_dtuple_rec_with_match_bytes.cpp
    cmp_get_pad_char.cpp
    cmp_dtuple_rec.cpp
    cmp_dtuple_is_prefix_of_rec.cpp
    cmp_rec_rec_simple_field.cpp
    cmp_rec_rec_simple.cpp
    cmp_rec_rec_with_match.cpp
    cmp_rec_rec.cpp
    cmp_dfield_dfield_eq_prefix.cpp
    test_cmp_data_data.cpp
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
    record
    PRIVATE
)

