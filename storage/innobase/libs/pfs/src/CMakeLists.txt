set( LIB_NAME pfs )

add_library( ${LIB_NAME} STATIC
    mysql_pfs_key_t.cpp
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
    univ
    PRIVATE
)

