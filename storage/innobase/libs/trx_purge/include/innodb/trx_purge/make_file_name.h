#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Build a standard undo tablespace file name from a space_id.
@param[in]	space_id	id of the undo tablespace.
@return file_name of the undo tablespace file */
char *make_file_name(space_id_t space_id);

}
