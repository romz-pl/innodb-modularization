#
# Warning flags for clang++ and g++ compilers
#

list( APPEND ROMZ_COMPILER_FLAGS -Wall )
list( APPEND ROMZ_COMPILER_FLAGS -Wextra )
list( APPEND ROMZ_COMPILER_FLAGS -Wpedantic )
list( APPEND ROMZ_COMPILER_FLAGS -Wdouble-promotion )
list( APPEND ROMZ_COMPILER_FLAGS -Wfloat-equal )
list( APPEND ROMZ_COMPILER_FLAGS -Wnon-virtual-dtor )
list( APPEND ROMZ_COMPILER_FLAGS -Wshadow )
list( APPEND ROMZ_COMPILER_FLAGS -Wunreachable-code )
list( APPEND ROMZ_COMPILER_FLAGS -Wconversion )
list( APPEND ROMZ_COMPILER_FLAGS -Wold-style-cast )
#list( APPEND ROMZ_COMPILER_FLAGS -Wpadded )
list( APPEND ROMZ_COMPILER_FLAGS -Wsign-conversion )
list( APPEND ROMZ_COMPILER_FLAGS -Wfloat-conversion )
list( APPEND ROMZ_COMPILER_FLAGS -Wzero-as-null-pointer-constant )
list( APPEND ROMZ_COMPILER_FLAGS -Woverloaded-virtual )
list( APPEND ROMZ_COMPILER_FLAGS -Wctor-dtor-privacy )

#
# Czy to zwraca poprawne warningi??
#
#list( APPEND ROMZ_COMPILER_FLAGS -Weffc++ )


#
# clang specific warning flags
#
if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    list( APPEND ROMZ_COMPILER_FLAGS -Wdeprecated )
    list( APPEND ROMZ_COMPILER_FLAGS -Wmissing-variable-declarations )
    list( APPEND ROMZ_COMPILER_FLAGS -Wunused-exception-parameter )
    list( APPEND ROMZ_COMPILER_FLAGS -Wweak-vtables )
    list( APPEND ROMZ_COMPILER_FLAGS -Wunused-member-function )
    list( APPEND ROMZ_COMPILER_FLAGS -Wunneeded-internal-declaration )
    list( APPEND ROMZ_COMPILER_FLAGS -Wundefined-func-template )
    list( APPEND ROMZ_COMPILER_FLAGS -Wshorten-64-to-32 )
    list( APPEND ROMZ_COMPILER_FLAGS -Wstring-conversion )
    list( APPEND ROMZ_COMPILER_FLAGS -Wreturn-std-move-in-c++11 )
endif()
