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
    assert

    # C
    compiler_hints

    # F
    formatting

    # U
    univ
)
