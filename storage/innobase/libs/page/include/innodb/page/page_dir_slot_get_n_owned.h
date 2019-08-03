#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_dir_slot_t.h>

/** Gets the number of records owned by a directory slot.
 @return number of records */
ulint page_dir_slot_get_n_owned(
    const page_dir_slot_t *slot); /*!< in: page directory slot */
