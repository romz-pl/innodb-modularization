set( LIB_NAME sync_latch )

add_library( ${LIB_NAME} STATIC
    CreateTracker.cpp
    create_tracker.cpp
    LatchCounter.cpp
    LatchMeta.cpp
    LatchMetaData.cpp
    LATCH_ADD_MUTEX.cpp
    LATCH_ADD_RWLOCK.cpp
    latch_id_t.cpp
    latch_level_t.cpp
    latch_meta.cpp
    latch_meta_t.cpp
    latch_t.cpp
    sync_basename.cpp
    sync_file_created_deregister.cpp
    sync_file_created_get.cpp
    sync_file_created_register.cpp
    sync_latch_get_counter.cpp
    sync_latch_get_level.cpp
    sync_latch_get_meta.cpp
    sync_latch_get_name.cpp
    sync_latch_get_name.cpp
    sync_latch_get_pfs_key.cpp
    sync_mutex_to_string.cpp
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

