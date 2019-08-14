#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct page_zip_des_t;

/** The following function is used to set the number of owned records.
@param[in,out]	rec		new-style physical record
@param[in,out]	page_zip	compressed page, or NULL
@param[in]	n_owned		the number of owned */
void rec_set_n_owned_new(rec_t *rec, page_zip_des_t *page_zip, ulint n_owned);
