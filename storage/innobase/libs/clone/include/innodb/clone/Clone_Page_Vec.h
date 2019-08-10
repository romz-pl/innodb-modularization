#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Page.h>

#include <vector>

/** Vector type for storing clone page IDs */
using Clone_Page_Vec = std::vector<Clone_Page>;
