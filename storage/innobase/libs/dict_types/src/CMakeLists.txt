set( LIB_NAME dict_types )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    index_id_t.cpp
    dict_err_ignore_t.cpp
    ib_quiesce_t.cpp
    DictSysMutex.cpp
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

