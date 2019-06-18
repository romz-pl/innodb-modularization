#
# Address sanitizer
#
option( ROMZ_ADDRESS_SANITIZER_ENABLED "Use Address Sanitizer" OFF )
if( ROMZ_ADDRESS_SANITIZER_ENABLED )

    # Always use compilers with debugging symbols
    list( APPEND ROMZ_COMPILER_FLAGS -g3 )

    # set options for sanitizer
    set( OPT -fno-omit-frame-pointer -fsanitize=address )

    list( APPEND ROMZ_COMPILER_FLAGS ${OPT} )
    list( APPEND ROMZ_LINKER_FLAGS ${OPT} )

endif()


#
# Undefined Behavior Sanitizer (UBSan)
#
option( ROMZ_UNDEFINED_BEHAVIOR_SANITIZER_ENABLED "Use Undefined Behavior Sanitizer" OFF )
if( ROMZ_UNDEFINED_BEHAVIOR_SANITIZER_ENABLED )

    # Always use compilers with debugging symbols
    list( APPEND ROMZ_COMPILER_FLAGS -g3 )

    # set options for sanitizer
    set( OPT -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=all -fsanitize-minimal-runtime )

    list( APPEND ROMZ_COMPILER_FLAGS ${OPT} )
    list( APPEND ROMZ_LINKER_FLAGS ${OPT} )

endif()


#
# Memory Sanitizer
#

#
# If you want MemorySanitizer to work properly and not produce any false positives,
# you must ensure that all the code in your program and in libraries it uses is
# instrumented (i.e. built with -fsanitize=memory). In particular, you would
# need to link against MSan-instrumented C++ standard library.
# We recommend to use libc++ for that purpose.
#
# For more details, see:
# https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
#
option( ROMZ_MEMORY_SANITIZER_ENABLED "Use Memory Behavior Sanitizer" OFF )
if( ROMZ_MEMORY_SANITIZER_ENABLED )

    # Always use compilers with debugging symbols
    list( APPEND ROMZ_COMPILER_FLAGS -g3 )

    # set options for sanitizer
    set( OPT -fno-omit-frame-pointer -fsanitize=memory )

    list( APPEND ROMZ_COMPILER_FLAGS ${OPT} )
    list( APPEND ROMZ_LINKER_FLAGS ${OPT} )

endif()
