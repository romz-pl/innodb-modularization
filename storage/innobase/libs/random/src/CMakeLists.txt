set( LIB_NAME random )

add_library( ${LIB_NAME} STATIC
    random.cpp
    ut_find_prime.cpp
    ut_rnd_ulint_counter.cpp
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

