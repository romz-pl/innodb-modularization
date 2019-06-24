set( LIB_NAME error )

add_library( ${LIB_NAME} STATIC
    dberr_t.cpp
    ut_error.cpp
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

