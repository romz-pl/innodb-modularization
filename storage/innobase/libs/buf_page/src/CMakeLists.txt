set( LIB_NAME buf_page )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    buf_io_fix.cpp
    buf_page_state.cpp
    buf_page_t.cpp
    buf_page_alloc_descriptor.cpp
    buf_page_get_state.cpp
    buf_page_in_file.cpp
    buf_flush_t.cpp
    buf_page_get_flush_type.cpp
    buf_page_set_flush_type.cpp
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
    log_sn
    page
    PRIVATE
)

