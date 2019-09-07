#pragma once

#include <innodb/univ/univ.h>

/** Returns TRUE if the roll pointer is of the insert type.
 @return true if insert undo log */
ibool trx_undo_roll_ptr_is_insert(roll_ptr_t roll_ptr); /*!< in: roll pointer */
