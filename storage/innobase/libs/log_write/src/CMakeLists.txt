set( LIB_NAME log_write )

add_library( ${LIB_NAME} STATIC
    Srv_cpu_usage.cpp
    srv_cpu_usage.cpp
    srv_log_spin_cpu_abs_lwm.cpp
    log_max_spins_when_waiting_in_user_thread.cpp
    srv_log_spin_cpu_pct_hwm.cpp
    srv_log_wait_for_write_spin_delay.cpp
    log_wait_for_write.cpp
    srv_log_wait_for_write_timeout.cpp
    validate_buffer.cpp
    validate_start_lsn.cpp
    compute_real_offset.cpp
    current_file_has_space.cpp
    start_next_file.cpp
    write_ahead_enough.cpp
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
    log_redo
    PRIVATE
)

