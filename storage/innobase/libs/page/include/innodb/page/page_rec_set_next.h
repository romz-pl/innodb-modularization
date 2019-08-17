#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Sets the pointer to the next record on the page.
@param[in]	rec	pointer to record, must not be page supremum
@param[in]	next	pointer to next record, must not be page infimum */
void page_rec_set_next(rec_t *rec, const rec_t *next);
