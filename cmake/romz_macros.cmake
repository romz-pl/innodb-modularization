
#
# Macros influencing source code
#

#
# Macro PRODUCTION
#
option( ROMZ_PRODUCTION_ENABLED "Enable some performace optimization defining PRODUCTION" OFF )
if( ROMZ_PRODUCTION_ENABLED )
    list( APPEND ROMZ_COMPILER_FLAGS -DPRODUCTION )
endif()




