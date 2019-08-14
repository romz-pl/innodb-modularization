#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_dir_slot_t.h>
#include <innodb/univ/rec_t.h>

/** Gets the record pointed to by a directory slot.
 @return pointer to record */
const rec_t *page_dir_slot_get_rec(
    const page_dir_slot_t *slot); /*!< in: directory slot */
