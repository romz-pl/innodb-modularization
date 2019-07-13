set( LIB_NAME ioasync )

add_library( ${LIB_NAME} STATIC
    AIO.cpp
    Slot.cpp
    os_aio_n_segments.cpp
    SRV_MAX_N_IO_THREADS.cpp
    srv_io_thread_function.cpp
    os_aio_segment_wait_events.cpp
    os_aio_validate.cpp
    os_last_printout.cpp
    AIOHandler.cpp
    LinuxAIOHandler.cpp
    srv_shutdown_t.cpp
    srv_shutdown_state.cpp
    buf_page_cleaner_is_active.cpp
    srv_io_thread_op_info.cpp
    srv_set_io_thread_op_info.cpp
    srv_reset_io_thread_op_info.cpp
    os_aio_func.cpp
    os_aio_simulated_handler.cpp
    SimulatedAIOHandler.cpp
    os_aio_linux_handler.cpp
    pfs_os_aio_func.cpp
    os_aio_handler.cpp
    os_aio_init.cpp
    os_aio_free.cpp
    os_aio_wake_all_threads_at_shutdown.cpp
    os_aio_wait_until_no_pending_writes.cpp
    os_aio_simulated_wake_handler_threads.cpp
    os_aio_recommend_sleep_for_read_threads.cpp
    os_aio_simulated_put_read_threads_to_sleep.cpp
    os_aio_print.cpp
    os_aio_refresh_stats.cpp
    os_aio_all_slots_free.cpp
    os_aio_print_pending_io.cpp
    os_aio.cpp
    os_file_set_size.cpp
    os_aio_validate_skip.cpp
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
    tablespace
    PRIVATE
)

