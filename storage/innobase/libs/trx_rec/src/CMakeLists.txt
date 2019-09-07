set( LIB_NAME trx_rec )

add_library( ${LIB_NAME} STATIC
    trx_undo_rec_copy.cpp
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
    trx_types
    PRIVATE
)

