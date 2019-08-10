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
    Arch_Grp_List.cpp
    Arch_Grp_List_Iter.cpp
    Arch_Log_Sys.cpp
    Arch_Block_Vec.cpp
    ArchPageData.cpp
    Arch_Page_Sys.cpp
    arch_log_sys.cpp
    arch_page_sys.cpp
    arch_remove_file.cpp
    log_archiver_thread_event.cpp
    arch_remove_dir.cpp
    page_archiver_thread_event.cpp
    page_archiver_is_active.cpp
    arch_init.cpp
    arch_free.cpp
    start_log_archiver_background.cpp
    log_archiver_thread.cpp
    Page_Arch_Cbk.cpp
    Page_Wait_Flush_Archiver_Cbk.cpp
    Page_Arch_Client_Ctx.cpp
    page_archiver_thread.cpp
    start_page_archiver_background.cpp
    pfs.cpp
    Arch_Recv_Group_Info.cpp
    Arch_Dir_Group_Info_Map.cpp
    Arch_Dblwr_Block.cpp
    Arch_Dblwr_Blocks.cpp
    Arch_Dblwr_Ctx.cpp
    Recv.cpp
    Log_Arch_Cbk.cpp
    Log_Arch_Client_Ctx.cpp
    srv_log_file_size.cpp
    srv_log_group_home_dir.cpp
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
    buffer
    crc32
    PRIVATE
)

