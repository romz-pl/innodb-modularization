#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Mark an undo number associated with a given space_id as unused and
available to be resused.  This happens when the fil_space_t is closed
associated with a drop undo tablespace.
@param[in] space_id  Undo Tablespace ID */
void unuse_space_id(space_id_t space_id);

}
