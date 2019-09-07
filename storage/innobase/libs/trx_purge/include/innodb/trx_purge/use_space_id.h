#pragma once

#include <innodb/univ/univ.h>

namespace undo {

/** Note that the undo space number for a space ID is being used.
Put that space_id into the space_id_bank.
@param[in] space_id  undo tablespace number */
void use_space_id(space_id_t space_id);

}
