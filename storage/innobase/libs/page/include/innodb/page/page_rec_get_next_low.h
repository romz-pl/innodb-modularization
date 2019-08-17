#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Gets the pointer to the next record on the page.
@param[in]	rec	pointer to record
@param[in]	comp	nonzero=compact page layout
@return pointer to next record */
const rec_t *page_rec_get_next_low(const rec_t *rec, ulint comp);
