set( LIB_NAME sync_event )

add_library( ${LIB_NAME} STATIC
    event_iter_t.cpp
    EventMutex.cpp
    os_cond_t.cpp
    os_event.cpp
    os_event_create.cpp
    os_event_destroy.cpp
    os_event_is_set.cpp
    os_event_list_t.cpp
    os_event_reset.cpp
    os_event_set.cpp
    os_event_t.cpp
    os_event_try_set.cpp
    os_event_wait.cpp
    os_event_wait_for.cpp
    os_event_wait_low.cpp
    os_event_wait_time.cpp
    os_event_wait_time_low.cpp
    OS_SYNC_INFINITE_TIME.cpp
    OS_SYNC_TIME_EXCEEDED.cpp
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
    sync_os
    wait
    PRIVATE
)

