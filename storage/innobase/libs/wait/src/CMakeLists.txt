set( LIB_NAME wait )

add_library( ${LIB_NAME} STATIC
    spin_wait_pause_multiplier.cpp
    ut_delay.cpp
    UT_RELAX_CPU.cpp
    ut_wait_for.cpp
    UT_WAIT_FOR.cpp
    Wait_stats.cpp
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
    time
    PRIVATE
)

