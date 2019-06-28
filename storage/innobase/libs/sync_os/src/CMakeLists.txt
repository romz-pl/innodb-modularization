set( LIB_NAME sync_os )

add_library( ${LIB_NAME} STATIC
    OSMutex.cpp
    sys_mutex_t.cpp
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
    logger
    PRIVATE
)

