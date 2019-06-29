set( LIB_NAME sync_mutex )

add_library( ${LIB_NAME} STATIC
    BlockFutexMutex.cpp
    BlockSpinMutex.cpp
    BlockSyncArrayMutex.cpp
    BlockSysMutex.cpp
    FutexMutex.cpp
    ib_bpmutex_t.cpp
    IB_mutex_guard.cpp
    ib_mutex_t.cpp
    mutex_create.cpp
    mutex_destroy.cpp
    mutex_enter.cpp
    mutex_enter_nospin.cpp
    mutex_enter_nowait.cpp
    mutex_exit.cpp
    mutex_free.cpp
    MUTEX_TYPE.cpp
    mutex_init.cpp
    mutex_own.cpp
    mutex_state_t.cpp
    mutex_validate.cpp
    MutexMonitor.cpp
    OSTrackMutex.cpp
    PolicyMutex.cpp
    SpinMutex.cpp
    srv_n_spin_wait_rounds.cpp
    srv_spin_wait_delay.cpp
    SyncArrayMutex.cpp
    SysMutex.cpp
    TTASEventMutex.cpp
    TTASFutexMutex.cpp
    TTASMutex.cpp
    UT_MUTEX_TYPE.cpp
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

