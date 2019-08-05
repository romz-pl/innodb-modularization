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
    mem_heap_create_block_func.cpp
    MEM_BLOCK_HEADER_SIZE.cpp
    mem_heap_create_block.cpp
    mem_heap_create_at.cpp
    mem_heap_block_free.cpp
    mem_heap_create_func.cpp
    mem_heap_create.cpp
    mem_heap_create_typed.cpp
    mem_heap_free.cpp
    mem_heap_zalloc.cpp
    mem_heap_alloc.cpp
    mem_heap_free_heap_top.cpp
    mem_heap_empty.cpp
    mem_heap_replace.cpp
    mem_heap_dup_replace.cpp
    mem_heap_strdup_replace.cpp
    mem_heap_free_top.cpp
    mem_heap_strdup.cpp
    mem_heap_strdupl.cpp
    mem_heap_strcat.cpp
    mem_heap_dup.cpp
    mem_heap_printf.cpp
    ut_strlcpy_rev.cpp
    mem_heap_printf_low.cpp
    mem_heap_add_block.cpp
    mem_heap_free_block_free.cpp
    mem_heap_allocator.cpp
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
    allocator
    lst
    PRIVATE
)

