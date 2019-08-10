#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Page.h>
#include <innodb/clone/Less_Clone_Page.h>

#include <set>

/** Set for storing unique page IDs. */
using Clone_Page_Set = std::set<Clone_Page, Less_Clone_Page>;
