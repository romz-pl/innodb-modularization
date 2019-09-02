set( LIB_NAME lock_rec )

add_library( ${LIB_NAME} STATIC
    RecID.cpp
    lock_get_min_heap_no.cpp
    lock_rec_get_first.cpp
    lock_clust_rec_some_has_impl.cpp
    Lock_iter.cpp
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
    lock_priv
    record
    PRIVATE
)

