set( LIB_NAME ioasync )

add_library( ${LIB_NAME} STATIC
    AIO.cpp
    AIO_mode.cpp
    Slot.cpp
    os_aio_n_segments.cpp
    SRV_MAX_N_IO_THREADS.cpp
    srv_io_thread_function.cpp
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
    io
    sync_mutex
    PRIVATE
)

