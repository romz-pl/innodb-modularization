#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Initialize the undo tablespace space_id bank which is a lock free
repository for information about the space IDs used for undo tablespaces.
It is used during creation in order to assign an unused space number and
during truncation in order to assign the next space_id within that
space_number range. */
void init_space_id_bank();

}
