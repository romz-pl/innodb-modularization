set( TEST_NAME log_block-utests )

add_executable( ${TEST_NAME}
    base.cpp
    main.cpp
)

add_test( NAME ${TEST_NAME} COMMAND ${TEST_NAME} )

target_compile_options( ${TEST_NAME} PRIVATE  )

target_include_directories( ${TEST_NAME} PRIVATE )

target_link_libraries( ${TEST_NAME} PRIVATE gtest log_block )


