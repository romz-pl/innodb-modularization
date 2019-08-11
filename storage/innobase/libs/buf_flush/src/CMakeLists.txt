set( LIB_NAME buf_flush )

add_library( ${LIB_NAME} STATIC
    buf_flush_list_mutex_enter.cpp
    buf_flush_list_mutex_exit.cpp
    buf_flush_list_mutex_own.cpp
    buf_flush_ready_for_replace.cpp
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
    buf_page
    PRIVATE
)

