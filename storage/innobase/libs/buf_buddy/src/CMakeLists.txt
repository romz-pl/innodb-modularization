set( LIB_NAME buf_buddy )

add_library( ${LIB_NAME} STATIC
    buf_buddy_free_t.cpp
    buf_buddy_stat_t.cpp
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
    buf_page
    PRIVATE
)

