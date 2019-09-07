#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Clear the s_under_construction vector. */
void clear_construction_list();

}
