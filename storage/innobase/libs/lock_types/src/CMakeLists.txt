set( LIB_NAME lock_types )

add_library( ${LIB_NAME} STATIC
    select_mode.cpp
    lock_mode.cpp
    lock_mode_string.cpp
    trx_lock_list_t.cpp
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
    error
    lst
    PRIVATE
)

