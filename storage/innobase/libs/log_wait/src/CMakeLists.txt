set( LIB_NAME log_wait )

add_library( ${LIB_NAME} STATIC
    Log_thread_waiting.cpp
    Srv_cpu_usage.cpp
    log_max_spins_when_waiting_in_user_thread.cpp
    log_wait_for_flush.cpp
    log_write_to_file_requests_are_frequent.cpp
    srv_cpu_usage.cpp
    srv_log_spin_cpu_abs_lwm.cpp
    srv_log_spin_cpu_pct_hwm.cpp
    srv_log_wait_for_flush_spin_delay.cpp
    srv_log_wait_for_flush_spin_hwm.cpp
    srv_log_wait_for_flush_timeout.cpp
    srv_log_wait_for_write_spin_delay.cpp
    srv_log_wait_for_write_timeout.cpp
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
    log_types
    monitor
    PRIVATE
)

