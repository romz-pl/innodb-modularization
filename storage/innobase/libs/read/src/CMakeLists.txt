set( LIB_NAME read )

add_library( ${LIB_NAME} STATIC
    MVCC.cpp
    ReadView.cpp
    ViewCheck.cpp
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
    dict_types
    trx_trx
    PRIVATE
)

