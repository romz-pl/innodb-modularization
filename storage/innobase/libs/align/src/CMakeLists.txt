set( LIB_NAME align )

add_library( ${LIB_NAME} STATIC
    ut_align.cpp
    ut_align_down.cpp
    ut_align_offset.cpp
    ut_calc_align_down.cpp
    ut_calc_align.cpp
    ut_uint64_align_down.cpp
    ut_uint64_align_up.cpp
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
    math
    PRIVATE
)

