set( LIB_NAME allocator )

add_library( ${LIB_NAME} STATIC
    aligned_array_pointer.cpp
    aligned_memory.cpp
    aligned_pointer.cpp
    alloc_max_retries.cpp
    MAP_ANON.cpp
    OUT_OF_MEMORY_MSG.cpp
    os_large_page_size.cpp
    os_mem_alloc_large.cpp
    os_mem_free_large.cpp
    os_total_large_mem_allocated.cpp
    os_use_large_pages.cpp
    psi.cpp
    ut_allocator.cpp
    ut_delete.cpp
    UT_DELETE.cpp
    ut_delete_array.cpp
    UT_DELETE_ARRAY.cpp
    ut_free.cpp
    ut_malloc.cpp
    ut_malloc_nokey.cpp
    UT_NEW.cpp
    UT_NEW_ARRAY.cpp
    UT_NEW_ARRAY_NOKEY.cpp
    ut_new_boot.cpp
    ut_new_boot_safe.cpp
    UT_NEW_NOKEY.cpp
    ut_new_pfx_t.cpp
    ut_realloc.cpp
    ut_zalloc.cpp
    ut_zalloc_nokey.cpp
    ut_zalloc_nokey_nofatal.cpp
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
    atomic
    counter
    memory_check
    PRIVATE
)

