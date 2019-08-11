set( LIB_NAME buf_chunk )

add_library( ${LIB_NAME} STATIC
    buf_chunk_init.cpp
    buf_chunk_map_reg.cpp
    buf_chunk_not_freed.cpp
    buf_chunk_t.cpp
    buf_get_nth_chunk_block.cpp
    buf_pool_chunk_map_t.cpp
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
    PRIVATE
)

