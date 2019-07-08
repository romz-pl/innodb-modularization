set( LIB_NAME monitor )

add_library( ${LIB_NAME} STATIC
    MONITOR_IS_ON.cpp
    MONITOR_OFF.cpp
    MONITOR_ON.cpp
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
    time
    PRIVATE
)

