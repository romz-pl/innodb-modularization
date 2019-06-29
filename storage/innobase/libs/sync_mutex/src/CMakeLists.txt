set( LIB_NAME sync_mutex )

add_library( ${LIB_NAME} STATIC
    mutex_state_t.cpp
    OSTrackMutex.cpp
    PolicyMutex.cpp
    TTASEventMutex.cpp
    TTASFutexMutex.cpp
    TTASMutex.cpp
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
    random
    sync_event
    sync_policy
    PRIVATE
)

