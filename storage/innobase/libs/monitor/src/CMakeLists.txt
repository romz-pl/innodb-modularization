set( LIB_NAME monitor )

add_library( ${LIB_NAME} STATIC
    MONITOR_ATOMIC_DEC.cpp
    MONITOR_ATOMIC_INC.cpp
    MONITOR_CHECK_DEFINED.cpp
    MONITOR_DEC.cpp
    MONITOR_DEC_NOCHECK.cpp
    MONITOR_DEC_VALUE.cpp
    MONITOR_FIELD.cpp
    MONITOR_INC.cpp
    MONITOR_INC_NOCHECK.cpp
    MONITOR_INC_TIME_IN_MICRO_SECS.cpp
    MONITOR_INC_VALUE.cpp
    MONITOR_INC_VALUE_CUMULATIVE.cpp
    MONITOR_INIT.cpp
    MONITOR_IS_ON.cpp
    MONITOR_LAST_VALUE.cpp
    MONITOR_MAX_MIN_NOT_INIT.cpp
    MONITOR_MAX_VALUE.cpp
    MONITOR_MAX_VALUE_START.cpp
    MONITOR_MIN_VALUE.cpp
    MONITOR_MIN_VALUE_START.cpp
    MONITOR_OFF.cpp
    MONITOR_ON.cpp
    MONITOR_RESET_ALL.cpp
    MONITOR_SAVE_LAST.cpp
    MONITOR_SAVE_START.cpp
    MONITOR_SET.cpp
    MONITOR_SET_DIFF.cpp
    MONITOR_SET_OFF.cpp
    MONITOR_SET_SIMPLE.cpp
    MONITOR_SET_START.cpp
    MONITOR_SET_UPD_MAX_ONLY.cpp
    MONITOR_START_VALUE.cpp
    MONITOR_STATUS.cpp
    MONITOR_VALUE.cpp
    MONITOR_VALUE_RESET.cpp
    MONITOR_VALUE_SINCE_START.cpp
    innodb_counter_value.cpp
    macros.cpp
    mon_option_t.cpp
    mon_type_t.cpp
    monitor_id_t.cpp
    monitor_info_t.cpp
    monitor_running_status.cpp
    monitor_running_t.cpp
    monitor_set_tbl.cpp
    monitor_type_t.cpp
    monitor_value_t.cpp
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
    atomic
    time
    PRIVATE
)

