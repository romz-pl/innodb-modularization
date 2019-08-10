set( LIB_NAME log_block )

add_library( ${LIB_NAME} STATIC
    log_block_calc_checksum.cpp
    log_block_calc_checksum_crc32.cpp
    log_block_calc_checksum_none.cpp
    log_block_convert_lsn_to_no.cpp
    log_block_get_checkpoint_no.cpp
    log_block_get_checksum.cpp
    log_block_get_data_len.cpp
    log_block_get_encrypt_bit.cpp
    log_block_get_first_rec_group.cpp
    log_block_get_flush_bit.cpp
    log_block_get_hdr_no.cpp
    log_block_set_checkpoint_no.cpp
    log_block_set_checksum.cpp
    log_block_set_data_len.cpp
    log_block_set_encrypt_bit.cpp
    log_block_set_first_rec_group.cpp
    log_block_set_flush_bit.cpp
    log_block_set_hdr_no.cpp
    log_block_store_checksum.cpp
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

