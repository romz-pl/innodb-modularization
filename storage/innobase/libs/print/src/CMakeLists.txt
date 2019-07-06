set( LIB_NAME print )

add_library( ${LIB_NAME} STATIC
    PrintBuffer.cpp
    ut_print_buf.cpp
    ut_print_buf_hex.cpp
    ut_print_timestamp.cpp
    ut_sprintf_timestamp.cpp
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
    formatting
    memory_check
    thread
    PRIVATE
)

