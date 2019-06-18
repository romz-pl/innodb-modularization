#
# Use gold linker instead GNU linker
#
list( APPEND ROMZ_LINKER_FLAGS -fuse-ld=gold )

#
# This linker's option is needed as well
#
list( APPEND ROMZ_LINKER_FLAGS -ldl )

