set( LIB_NAME buf_frame )

add_library( ${LIB_NAME} STATIC
    buf_frame_align.cpp
    buf_frame_copy.cpp
    buf_frame_t.cpp
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
    align
    PRIVATE
)

