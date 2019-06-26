set( LIB_NAME formatting )

add_library( ${LIB_NAME} STATIC
    formatting.cpp
    get_first_format.cpp
    hex.cpp
    ut_vsnprintf.cpp
    verify_fmt_match.cpp
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

