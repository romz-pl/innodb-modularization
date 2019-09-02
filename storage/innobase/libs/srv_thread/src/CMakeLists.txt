set( LIB_NAME srv_thread )

add_library( ${LIB_NAME} STATIC
    srv_thread_type.cpp
    srv_slot_t.cpp
    Srv_threads.cpp
    srv_threads.cpp
    srv_thread_type_validate.cpp
    srv_slot_get_type.cpp
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
    sync_event
    PRIVATE
)

