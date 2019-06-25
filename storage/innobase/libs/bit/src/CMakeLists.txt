set( LIB_NAME bit )

add_library( ${LIB_NAME} STATIC
    UT_BITS_IN_BYTES.cpp
    ut_bit_get_nth.cpp
    ut_bit_set_nth.cpp
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

