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
    #bit
    #buf_calc
    #buf_lru

    # C
    clone
    cmp
    #compiler_hints
    #counter
    #crc32

    # D
    #data_types
    #dict
    #dict_types

    # E
    #error

    # F
    #formatting

    # H
    #hash

    # I
    #io
    #ioasync

    # L
    lock_rec
    #log_arch
    #log_closer
    #log_recv
    #log_sys
    #log_types
    #log_wait
    #log_write
    #logger
    #lst

    # M
    #machine
    #math
    #memory
    #memory_check
    #monitor
    #mtr

    # P
    #page
    #print

    # R
    #random
    rbt
    read
    #record

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
    #tablespace
    #thread
    #time
    #trx_sys

    # U
    #univ

    #W
    #wait
)
