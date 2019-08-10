set( LIB_NAME log_files )

add_library( ${LIB_NAME} STATIC
    log_allocate_file_header_buffers.cpp
    log_deallocate_file_header_buffers.cpp
    log_files_header_fill.cpp
    log_files_header_flush.cpp
    log_files_real_offset.cpp
    log_files_real_offset_for_lsn.cpp
    log_files_size_offset.cpp
    log_files_update_offsets.cpp
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
    log_block
    tablespace
    PRIVATE
)

