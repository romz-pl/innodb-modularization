set( LIB_NAME dict )

add_library( ${LIB_NAME} STATIC
    id_name_t.cpp
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
    compiler_hints
    PRIVATE
)

