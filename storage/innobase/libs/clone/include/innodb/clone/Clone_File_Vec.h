#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_File_Meta.h>

#include <vector>

/** Vector type for storing clone files */
using Clone_File_Vec = std::vector<Clone_File_Meta *>;
