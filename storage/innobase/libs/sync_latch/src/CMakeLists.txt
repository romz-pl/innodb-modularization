set( LIB_NAME sync_latch )

add_library( ${LIB_NAME} STATIC
    LatchCounter.cpp
    LatchMeta.cpp
    LatchMetaData.cpp
    latch_id_t.cpp
    latch_level_t.cpp
    latch_meta_t.cpp
    latch_t.cpp
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
    allocator
    sync_os
    PRIVATE
)

