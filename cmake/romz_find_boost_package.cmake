#
# finds the boost package with the minimium vesion
#
find_package( Boost 1.62 REQUIRED COMPONENTS
    date_time
    filesystem
    iostreams
    program_options
    system
    thread
    program_options
)

#
# Remove deprecated auto_pointer from boost library
#
list( APPEND ROMZ_COMPILER_FLAGS -DBOOST_NO_AUTO_PTR )

