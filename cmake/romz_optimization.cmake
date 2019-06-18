#
# Optimization level
#
option( ROMZ_OPTIMIZATION_ENABLED "Enable compiler optimization" OFF )
if( ROMZ_OPTIMIZATION_ENABLED )
    list( APPEND ROMZ_COMPILER_FLAGS -DNDEBUG -O2 )
else()
    list( APPEND ROMZ_COMPILER_FLAGS -O0 )
endif()

