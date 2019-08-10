set( LIB_NAME log_buffer )

add_library( ${LIB_NAME} STATIC
    log_allocate_buffer.cpp
    log_buffer_close.cpp
    log_buffer_dirty_pages_added_up_to_lsn.cpp
    log_buffer_flush_order_lag.cpp
    log_buffer_flush_to_disk.cpp
    log_buffer_get_last_block.cpp
    log_buffer_ready_for_write_lsn.cpp
    log_buffer_resize.cpp
    log_buffer_resize_low.cpp
    log_buffer_set_first_record_group.cpp
    log_buffer_s_lock_enter.cpp
    log_buffer_s_lock_exit.cpp
    log_buffer_write_completed.cpp
    log_buffer_write.cpp
    log_buffer_x_lock_own.cpp
    log_calc_buf_size.cpp
    log_deallocate_buffer.cpp
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
    log_block
    monitor
    PRIVATE
)

