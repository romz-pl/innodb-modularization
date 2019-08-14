set( LIB_NAME gis_type )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    node_seq_t.cpp
    node_visit_t.cpp
    rtr_info_active.cpp
    rtr_info_t.cpp
    rtr_info_track_t.cpp
    rtr_mbr_t.cpp
    rtr_node_path_t.cpp
    rtr_rec_move_t.cpp
    rtr_rec_t.cpp
    rtr_rec_vector.cpp
    rtr_ssn_t.cpp
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
    memory
    sync_mutex
    PRIVATE
)

