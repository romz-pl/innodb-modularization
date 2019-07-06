set( LIB_NAME memory )

add_library( ${LIB_NAME} STATIC
    macros.cpp
    mem_block_get_free.cpp
    mem_block_get_len.cpp
    mem_block_get_start.cpp
    mem_block_get_type.cpp
    mem_block_info_t.cpp
    mem_block_set_free.cpp
    mem_block_set_len.cpp
    mem_block_set_start.cpp
    mem_block_set_type.cpp
    mem_block_t.cpp
    mem_block_validate.cpp
    mem_heap_create_block_func.cpp
    mem_heap_get_size.cpp
    mem_heap_t.cpp
    mem_heap_validate.cpp
    ut_memcmp.cpp
    ut_memcpy.cpp
    ut_memmove.cpp
    mem_heap_get_heap_top.cpp
    mem_heap_is_top.cpp
    validate_no_mans_land.cpp
    mem_heap_get_top.cpp
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
    align
    lst
    logger
    PRIVATE
)

