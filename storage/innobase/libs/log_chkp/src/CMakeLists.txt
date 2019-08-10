set( LIB_NAME log_chkp )

add_library( ${LIB_NAME} STATIC
    log_allocate_checkpoint_buffer.cpp
    log_checkpointer_mutex_enter.cpp
    log_checkpointer_mutex_exit.cpp
    log_checkpointer_mutex_own.cpp
    log_deallocate_checkpoint_buffer.cpp
    log_get_checkpoint_age.cpp
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

