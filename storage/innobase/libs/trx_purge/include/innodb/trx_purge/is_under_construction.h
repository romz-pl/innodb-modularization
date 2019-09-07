#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Is an undo tablespace under constuction at the moment.
@param[in]	space_id	space id to check
@return true if marked for truncate, else false. */
bool is_under_construction(space_id_t space_id);

}
