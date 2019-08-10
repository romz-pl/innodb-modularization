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
    PRIVATE
)

