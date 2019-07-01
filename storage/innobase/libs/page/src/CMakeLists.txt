set( LIB_NAME page )

add_library( ${LIB_NAME} STATIC
    flag.cpp
    header.cpp
    page_no_t.cpp
    page_size_t.cpp
    page_type_t.cpp
    page_t.cpp
    univ_page_size.cpp
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
    machine
    math
    PRIVATE
)

