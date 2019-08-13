set( LIB_NAME dict_types )

add_library( ${LIB_NAME} STATIC
    DictSysMutex.cpp
    dict_err_ignore_t.cpp
    flags.cpp
    ib_quiesce_t.cpp
    id_name_t.cpp
    index_id_t.cpp
    table_name_t.cpp
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
    sync_mutex
    PRIVATE
)

