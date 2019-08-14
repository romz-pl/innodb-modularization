set( LIB_NAME lock_priv )

add_library( ${LIB_NAME} STATIC
    lock_table_t.cpp
    lock_rec_t.cpp
    lock_t.cpp
    flags.cpp
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
    hash
    lock_types
    trx_types
    PRIVATE
)

