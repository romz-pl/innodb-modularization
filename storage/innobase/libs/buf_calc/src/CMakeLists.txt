set( LIB_NAME buf_calc )

add_library( ${LIB_NAME} STATIC
    BlockReporter.cpp
    buf_calc_page_crc32.cpp
    buf_calc_page_new_checksum.cpp
    buf_calc_page_old_checksum.cpp
    buf_checksum_algorithm_name.cpp
    flags.cpp
    srv_checksum_algorithm.cpp
    srv_checksum_algorithm_t.cpp
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
    crc32
    page
    PRIVATE
)

