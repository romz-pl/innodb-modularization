set( LIB_NAME buf_lru )

add_library( ${LIB_NAME} STATIC
    buf_LRU_stat_inc_unzip.cpp
    buf_LRU_stat_t.cpp
    buf_LRU_stat_cur.cpp
    buf_LRU_stat_sum.cpp
    buf_LRU_stat_inc_io.cpp
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
    buf_page
    PRIVATE
)

