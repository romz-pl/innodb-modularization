set( LIB_NAME lock_sys )

add_library( ${LIB_NAME} STATIC
    LockMutex.cpp
    lock_mutex_enter.cpp
    lock_mutex_enter_nowait.cpp
    lock_mutex_exit.cpp
    lock_mutex_own.cpp
    lock_sys.cpp
    lock_sys_t.cpp
    lock_wait_mutex_enter.cpp
    lock_wait_mutex_exit.cpp
    lock_wait_mutex_own.cpp
    lock_sys_create.cpp
    lock_latest_err_file.cpp
    lock_sys_resize.cpp
    lock_sys_close.cpp
    lock_cached_lock_mode_names.cpp
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
    io
    hash
    lock_priv
    srv_thread
    sync_mutex
    PRIVATE
)

