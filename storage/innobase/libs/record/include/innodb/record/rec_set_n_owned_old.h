#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the number of owned records.
@param[in]	rec		old-style physical record
@param[in]	n_owned		the number of owned */
void rec_set_n_owned_old(rec_t *rec, ulint n_owned);
