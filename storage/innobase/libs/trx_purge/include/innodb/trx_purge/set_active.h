#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Set an undo tablespace active. */
void set_active(space_id_t space_id);

}
