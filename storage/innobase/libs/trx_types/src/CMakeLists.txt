set( LIB_NAME trx_types )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    trx_que_t.cpp
    trx_state_t.cpp
    trx_dict_op_t.cpp
    trx_savept_t.cpp
    trx_sysf_t.cpp
    trx_rsegsf_t.cpp
    trx_rsegf_t.cpp
    trx_usegf_t.cpp
    trx_ulogf_t.cpp
    trx_upagef_t.cpp
    trx_undo_rec_t.cpp
    RsegMutex.cpp
    TrxMutex.cpp
    UndoMutex.cpp
    PQMutex.cpp
    TrxSysMutex.cpp
    trx_rseg_t.cpp
    Rsegs_Vector.cpp
    Rseg_Iterator.cpp
    Rsegs.cpp
    TrxUndoRsegs.cpp
    purge_pq_t.cpp
    trx_ids_t.cpp
    TrxTrack.cpp
    TrxTrackHash.cpp
    TrxTrackHashCmp.cpp
    TrxTrackCmp.cpp
    TrxIdSet.cpp
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
    lst
    sync_rw
    PRIVATE
)

