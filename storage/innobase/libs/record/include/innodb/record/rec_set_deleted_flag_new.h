#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct page_zip_des_t;

/** The following function is used to set the deleted bit.
@param[in,out]	rec		new-style physical record
@param[in,out]	page_zip	compressed page, or NULL
@param[in]	flag		nonzero if delete marked */
void rec_set_deleted_flag_new(rec_t *rec, page_zip_des_t *page_zip, ulint flag);
