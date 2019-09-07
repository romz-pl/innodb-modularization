set( LIB_NAME trx_roll )

add_library( ${LIB_NAME} STATIC
    roll_node_create.cpp
    roll_node_state.cpp
    roll_node_t.cpp
    trx_is_recv.cpp
    trx_recovery_rollback_thread.cpp
    trx_release_savepoint_for_mysql.cpp
    trx_roll_check_undo_rec_ordering.cpp
    trx_roll_pop_top_rec_of_trx.cpp
    trx_roll_savepoints_free.cpp
    trx_rollback_for_mysql.cpp
    trx_rollback_last_sql_stat_for_mysql.cpp
    trx_rollback_or_clean_is_active.cpp
    trx_rollback_or_clean_recovered.cpp
    trx_rollback_step.cpp
    trx_rollback_to_savepoint.cpp
    trx_rollback_to_savepoint_for_mysql.cpp
    trx_savepoint_for_mysql.cpp
    trx_savept_take.cpp
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
    memory
    que_types
    trx_types
    PRIVATE
)

