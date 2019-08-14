set( LIB_NAME disk )

add_library( ${LIB_NAME} STATIC
    fil_page_get_next.cpp
    fil_page_get_prev.cpp
    fil_page_get_type.cpp
    fil_page_index_page_check.cpp
    fil_page_set_type.cpp
    fil_page_type_is_index.cpp
    flags.cpp
    page_size_t.cpp
    page_t.cpp
    page_type_t.cpp
    univ_page_size.cpp
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
    machine
    math
    PRIVATE
)

