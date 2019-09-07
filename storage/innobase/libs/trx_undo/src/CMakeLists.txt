set( LIB_NAME assert )

add_library( ${LIB_NAME} STATIC
    assert.cpp
    ASSERT_ZERO.cpp
    ASSERT_ZERO_BLOB.cpp
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
    compiler_hints
    PRIVATE
)

