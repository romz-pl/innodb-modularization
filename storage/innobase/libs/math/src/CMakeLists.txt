set( LIB_NAME math )

add_library( ${LIB_NAME} STATIC
    ut_2_exp.cpp
    ut_2_log.cpp
    ut_2_power_up.cpp
    ut_2pow_remainder.cpp
    ut_2pow_round.cpp
    ut_pair_cmp.cpp
    ut_pair_min.cpp
    ut_ulint_cmp.cpp
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
    assert
    PRIVATE
)

