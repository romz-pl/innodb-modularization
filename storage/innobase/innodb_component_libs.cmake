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

    # B
    bit

    # C
    compiler_hints

    # E
    error

    # F
    formatting

    # L
    logger

    # M
    math

    # S
    string

    # T
    time

    # U
    univ
)
