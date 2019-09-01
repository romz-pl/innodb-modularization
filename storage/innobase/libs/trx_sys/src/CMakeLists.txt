set( LIB_NAME trx_sys )

add_library( ${LIB_NAME} STATIC
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
    trx_types
    PRIVATE
)

