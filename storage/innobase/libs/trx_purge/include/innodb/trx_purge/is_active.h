#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/* Return whether the undo tablespace is active.  If this is a
non-undo tablespace, then it will not be found in spaces and it
will not be under construction, so this function will return true.
@param[in]  space_id   Undo Tablespace ID
@param[in]  get_latch  Specifies whether the rsegs->s_lock() is needed.
@return true if active (non-undo spaces are always active) */
bool is_active(space_id_t space_id, bool get_latch = true);

}
