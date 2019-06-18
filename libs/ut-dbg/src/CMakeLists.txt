set( LIB_NAME ut-dbg )

add_library( ${LIB_NAME} STATIC
    ut_a.cc
    ut_ad.cc
    ut_d.cc
    ut_dbg_assertion_failed.cc
)

target_compile_options( ${LIB_NAME} PRIVATE ${ROMZ_COMPILER_FLAGS} )

target_include_directories(
    ${LIB_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(
    ${LIB_NAME}
    PUBLIC
    # here add public libraries
    PRIVATE
    # here add private libraries
)

