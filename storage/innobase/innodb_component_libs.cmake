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
    align
    assert
    atomic

    # B
    bit

    # C
    compiler_hints
    counter

    # E
    error

    # F
    formatting

    # L
    logger

    # M
    math
    memory

    # R
    random

    # S
    string

    # T
    thread
    time

    # U
    univ

    #W
    wait
)
