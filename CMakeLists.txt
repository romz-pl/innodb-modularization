cmake_minimum_required( VERSION 3.5 )
project( innodb-modularization )

#
# list of directories specifying a search path for CMake modules to be loaded by the include() or find_package()
# commands before checking the default modules that come with CMake.
#
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" )

#
# Variable collectiong all compliation flags
#
set( ROMZ_COMPILER_FLAGS "" )

#
# Variable collectiong all linking flags
#
set( ROMZ_LINKER_FLAGS "" )

#
# Set compilation and linking flags
#
include( romz_cxx_version )
include( romz_warnings )
include( romz_debugging )
include( romz_optimization )
include( romz_macros )
include( romz_code_coverage )
include( romz_sanitizer )
include( romz_linker_version )
include( romz_gtest )
include( romz_ctest )

#
#
add_subdirectory( 3rdparty )
add_subdirectory( libs )
# add_subdirectory( tests )

