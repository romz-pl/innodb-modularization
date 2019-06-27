set( LIB_NAME memory_check )

add_library( ${LIB_NAME} STATIC
    memory_check.cpp
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

