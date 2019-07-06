#
# This cmake define the set of component libraries
#

ADD_SUBDIRECTORY( 3rdparty )
ADD_SUBDIRECTORY( libs )

#
# Define the libraries needed to build following targets:
#    innodb_zipdecompress
#    innobase
#
# List in alphabetical order
#
SET( INNODB_COMPONENT_LIBS
    # A
    #align
    #allocator
    #assert
    #atomic

    # B
    bit

    # C
    #compiler_hints
    #counter

    # E
    #error

    # F
    #formatting

    # I
    #io
    ioasync

    # L
    #logger
    #lst

    # M
    #machine
    #math
    #memory
    #memory_check

    # P
    #page

    # R
    #random
    record

    # S
    #string
    sync_array
    #sync_event
    #sync_latch
    #sync_mutex
    #sync_os
    #sync_policy
    #sync_rw

    # T
    tablespace
    #thread
    #time

    # U
    #univ

    #W
    #wait
)
