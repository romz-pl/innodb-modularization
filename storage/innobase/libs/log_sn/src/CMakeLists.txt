set( LIB_NAME log_sn )

add_library( ${LIB_NAME} STATIC
    LSN_PF.cpp
    atomic_lsn_t.cpp
    atomic_sn_t.cpp
    flags.cpp
    log_lsn_validate.cpp
    log_translate_lsn_to_sn.cpp
    log_translate_sn_to_lsn.cpp
    lsn_t.cpp
    sn_t.cpp
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
    disk
    formatting
    PRIVATE
)

