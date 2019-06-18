#
# Code coverage
#
if( ROMZ_CODE_COVERAGE_ENABLED )
    list( APPEND ROMZ_COMPILER_FLAGS -fprofile-arcs -ftest-coverage )
    list( APPEND ROMZ_LINKER_FLAGS -fprofile-arcs )
endif()

