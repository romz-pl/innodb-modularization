#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Gets the pointer to the next record on the page.
 @return pointer to next record */
const rec_t *page_rec_get_next_const(
    const rec_t *rec); /*!< in: pointer to record */
