set( LIB_NAME clone )

add_library( ${LIB_NAME} STATIC
    flags.cpp
    Snapshot_State.cpp
    Clone_Desc_Type.cpp
    Clone_Desc_Header.cpp
    Clone_Task_Meta.cpp
    Chunk_Map.cpp
    Chnunk_Bitmap.cpp
    Chunk_Info.cpp
    Clone_Desc_Locator.cpp
    Clone_Desc_Task_Meta.cpp
    Clone_Desc_State.cpp
    Clone_File_Meta.cpp
    Clone_Desc_File_MetaData.cpp
    Clone_Desc_Data.cpp
    choose_desc_version.cpp
    clone_validate_locator.cpp
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

