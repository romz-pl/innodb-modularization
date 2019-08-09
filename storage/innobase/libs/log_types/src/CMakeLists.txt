set( LIB_NAME log_types )

add_library( ${LIB_NAME} STATIC
    lsn_t.cpp
    atomic_lsn_t.cpp
    sn_t.cpp
    atomic_sn_t.cpp
    checkpoint_no_t.cpp
    log_flushes_t.cpp
    log_checksum_func_t.cpp
    Log_clock.cpp
    Log_clock_point.cpp
    log_header_format_t.cpp
    log_state_t.cpp
    log_lock_no_t.cpp
    Log_handle.cpp
    log_t.cpp
    Link_buf.cpp
    LSN_PF.cpp
    mlog_id_t.cpp
    recv_recovery_is_on.cpp
    recv_recovery_on.cpp
    log_sys.cpp
    log_get_checkpoint_lsn.cpp
    flags.cpp
    log_translate_sn_to_lsn.cpp
    log_get_lsn.cpp
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
    disk
    sync_rw
    PRIVATE
)

