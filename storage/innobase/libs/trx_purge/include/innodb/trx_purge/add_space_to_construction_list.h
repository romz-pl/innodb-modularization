#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Add undo tablespace to s_under_construction vector.
@param[in]	space_id	space id of tablespace to
truncate */
void add_space_to_construction_list(space_id_t space_id);

}
