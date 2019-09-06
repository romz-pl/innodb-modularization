set( LIB_NAME que_types )

add_library( ${LIB_NAME} STATIC
    que_common_t.cpp
    que_node_t.cpp
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
    data_types
    PRIVATE
)

