#
# Debugging level
#

# set( ROMZ_DEBUGGING_ENABLED ON )

option( ROMZ_DEBUGGING_ENABLED "Enable debugging mode" OFF )
if( ROMZ_DEBUGGING_ENABLED )
    list( APPEND ROMZ_COMPILER_FLAGS -g3 )
endif()

