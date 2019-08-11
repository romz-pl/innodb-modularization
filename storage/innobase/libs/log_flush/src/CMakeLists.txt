set( LIB_NAME log_flush )

add_library( ${LIB_NAME} STATIC
    log_allocate_flush_events.cpp
    log_deallocate_flush_events.cpp
    log_flusher_mutex_enter.cpp
    log_flusher_mutex_enter_nowait.cpp
    log_flusher_mutex_exit.cpp
    log_flusher_mutex_own.cpp
    log_flush_notifier_mutex_enter.cpp
    log_flush_notifier_mutex_exit.cpp
    log_flush_notifier_mutex_own.cpp
    srv_log_flush_events.cpp

    srv_flush_log_at_timeout.cpp
    srv_flush_log_at_trx_commit.cpp
    srv_flushing_avg_loops.cpp
    srv_log_flush_notifier_spin_delay.cpp
    srv_log_flush_notifier_timeout.cpp
    srv_log_flusher_spin_delay.cpp
    srv_log_flusher_timeout.cpp

    log_flush_low.cpp
    log_flush_notifier.cpp
    log_flush_update_stats.cpp
    log_flusher.cpp
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
    log_write
    PRIVATE
)

