#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Build a standard undo tablespace name from a space_id.
@param[in]	space_id	id of the undo tablespace.
@return tablespace name of the undo tablespace file */
char *make_space_name(space_id_t space_id);

}
