set( LIB_NAME buf_page )

add_library( ${LIB_NAME} STATIC
    buf_flush_t.cpp
    buf_io_fix.cpp
    buf_page_alloc_descriptor.cpp
    buf_page_belongs_to_unzip_LRU.cpp
    buf_page_can_relocate.cpp
    buf_page_free_descriptor.cpp
    buf_page_get_block.cpp
    buf_page_get_flush_type.cpp
    buf_page_get_freed_page_clock.cpp
    buf_page_get_io_fix.cpp
    buf_page_get_io_fix_unlocked.cpp
    buf_page_get_mutex.cpp
    buf_page_get_newest_modification.cpp
    buf_page_get_state.cpp
    buf_page_in_file.cpp
    buf_page_is_accessed.cpp
    buf_page_is_old.cpp
    buf_page_mutex_enter.cpp
    buf_page_mutex_exit.cpp
    buf_page_mutex_own.cpp
    buf_page_print_flags.cpp
    buf_page_release_latch.cpp
    buf_page_release_zip.cpp
    buf_page_set_accessed.cpp
    buf_page_set_flush_type.cpp
    buf_page_set_io_fix.cpp
    buf_page_set_old.cpp
    buf_page_set_state.cpp
    buf_page_set_sticky.cpp
    buf_page_state.cpp
    buf_page_t.cpp
    buf_page_unset_sticky.cpp
    flags.cpp
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
    buf_calc
    buf_pool
    log_sn
    PRIVATE
)

