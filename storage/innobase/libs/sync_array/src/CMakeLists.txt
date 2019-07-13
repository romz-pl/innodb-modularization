set( LIB_NAME sync_array )

add_library( ${LIB_NAME} STATIC
    BlockWaitMutex.cpp
    WaitMutex.cpp
    sync_arr_cell_can_wake_up.cpp
    sync_arr_wake_threads_if_sema_free.cpp
    sync_array_cell_print.cpp
    sync_array_close.cpp
    sync_array_deadlock_step.cpp
    sync_array_detect_deadlock.cpp
    sync_array_enter.cpp
    sync_array_exit.cpp
    sync_array_find_thread.cpp
    sync_array_free.cpp
    sync_array_free_cell.cpp
    sync_array_get_and_reserve_cell.cpp
    sync_array_get_nth_cell.cpp
    sync_array_init.cpp
    sync_array_object_signalled.cpp
    sync_array_print.cpp
    sync_array_print_info.cpp
    sync_array_print_info_low.cpp
    sync_array_print_long_waits.cpp
    sync_array_print_long_waits_low.cpp
    sync_array_report_error.cpp
    sync_array_reserve_cell.cpp
    sync_array_t.cpp
    sync_array_validate.cpp
    sync_array_wait_event.cpp
    sync_array_wake_threads_if_sema_free_low.cpp
    sync_cell_get_event.cpp
    sync_cell_t.cpp
    sync_object_t.cpp
    sync_array_get.cpp
    sync_array_size.cpp
    sync_wait_array.cpp
    srv_sync_array_size.cpp
    sg_count.cpp
    sync_print.cpp
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
    string
    sync_rw
    PRIVATE
)

