set( LIB_NAME thread )

add_library( ${LIB_NAME} STATIC
    os_thread_create.cpp
    os_thread_eq.cpp
    os_thread_get_curr_id.cpp
    os_thread_handle.cpp
    os_thread_id_t.cpp
    os_thread_set_priority.cpp
    os_thread_sleep.cpp
    os_thread_yield.cpp
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
    logger
    pfs
    PRIVATE
)

