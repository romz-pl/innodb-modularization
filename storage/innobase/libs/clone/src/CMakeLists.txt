set( LIB_NAME clone )

add_library( ${LIB_NAME} STATIC
    Chnunk_Bitmap.cpp
    Chunk_Info.cpp
    Chunk_Map.cpp
    Clone_Desc_Data.cpp
    Clone_Desc_File_MetaData.cpp
    Clone_Desc_Header.cpp
    Clone_Desc_Locator.cpp
    Clone_Desc_State.cpp
    Clone_Desc_Task_Meta.cpp
    Clone_Desc_Type.cpp
    Clone_File_Map.cpp
    Clone_File_Meta.cpp
    Clone_File_Vec.cpp
    Clone_Handle.cpp
    Clone_Handle_State.cpp
    Clone_Handle_Type.cpp
    Clone_Min.cpp
    Clone_Monitor.cpp
    Clone_Msec.cpp
    Clone_Page.cpp
    Clone_Page_Set.cpp
    Clone_Page_Vec.cpp
    Clone_Sec.cpp
    Clone_Snapshot.cpp
    Clone_Sys.cpp
    Clone_Sys_State.cpp
    Clone_System_State.cpp
    Clone_Task.cpp
    Clone_Task_Manager.cpp
    Clone_Task_Meta.cpp
    Clone_Task_State.cpp
    Less_Clone_Page.cpp
    Snapshot_State.cpp
    add_page_callback.cpp
    add_redo_file_callback.cpp
    add_redo_file_callback.cpp
    choose_desc_version.cpp
    clone_free.cpp
    clone_init.cpp
    clone_mark_abort.cpp
    clone_mark_active.cpp
    clone_sys.cpp
    clone_validate_locator.cpp
    fix_cloned_tables.cpp
    flags.cpp
    innodb_clone_ack.cpp
    innodb_clone_apply.cpp
    innodb_clone_apply_begin.cpp
    innodb_clone_apply_end.cpp
    innodb_clone_begin.cpp
    innodb_clone_copy.cpp
    innodb_clone_end.cpp
    innodb_clone_get_capability.cpp
    is_ddl_temp_table.cpp
    pfs.cpp
    print_chunk_info.cpp
    srv_redo_log_encrypt.cpp
    srv_undo_log_encrypt.cpp
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
    log_arch
    PRIVATE
)

