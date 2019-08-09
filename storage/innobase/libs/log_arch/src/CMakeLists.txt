set( LIB_NAME log_arch )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    log_archiver_thread_event.cpp
    log_archiver_is_active.cpp
    Arch_Client_State.cpp
    Arch_State.cpp
    Arch_Blk_State.cpp
    Arch_Blk_Type.cpp
    Arch_Blk_Flush_Type.cpp
    Arch_Page_Dblwr_Offset.cpp
    Arch_Page_Pos.cpp
    Arch_Point.cpp
    Arch_Reset_File.cpp
    Arch_Reset.cpp
    Arch_Block.cpp
    Arch_File_Ctx.cpp
    Arch_Group.cpp
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
    log_types
    PRIVATE
)

