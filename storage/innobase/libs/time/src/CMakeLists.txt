set( LIB_NAME time )

add_library( ${LIB_NAME} STATIC
    ib_time_t.cpp
    ut_time.cpp
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
    assert
    PRIVATE
)

