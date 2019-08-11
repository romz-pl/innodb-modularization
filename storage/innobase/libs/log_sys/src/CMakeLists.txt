set( LIB_NAME log_sys )

add_library( ${LIB_NAME} STATIC
    log_sys_close.cpp
    log_sys_init.cpp
    log_sys_object.cpp
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
    log_closer
    PRIVATE
)

