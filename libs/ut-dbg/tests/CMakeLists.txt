set( TEST_NAME ut-dbg-utests )

add_executable( ${TEST_NAME}
    basic.cpp
    main.cpp
)

add_test( NAME ${TEST_NAME} COMMAND ${TEST_NAME} )

target_compile_options( ${TEST_NAME} PRIVATE ${ROMZ_COMPILER_FLAGS} )

target_link_libraries( ${TEST_NAME} PRIVATE ut-dbg ${ROMZ_LINKER_FLAGS} )
