#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Return the next available undo space ID to be used for a new explicit
undo tablespaces.
@retval if success, next available undo space number.
@retval if failure, SPACE_UNKNOWN */
space_id_t get_next_available_space_num();

}
