set( LIB_NAME log_block )

add_library( ${LIB_NAME} STATIC
    log_block_calc_checksum.cpp
    log_block_calc_checksum_crc32.cpp
    log_block_calc_checksum_none.cpp
    log_block_get_checksum.cpp
    log_block_set_checksum.cpp
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
    log_types
    PRIVATE
)

