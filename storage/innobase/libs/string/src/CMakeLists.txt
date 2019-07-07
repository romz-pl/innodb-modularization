set( LIB_NAME string )

add_library( ${LIB_NAME} STATIC
    innobase_basename.cpp
    mem_strdup.cpp
    ut_raw_to_hex.cpp
    ut_str_sql_format.cpp
    ut_str3cat.cpp
    ut_strcmp_functor.cpp
    ut_strcmp.cpp
    ut_strcpy.cpp
    ut_strlcpy.cpp
    ut_strlen.cpp
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
    allocator
    PRIVATE
)

