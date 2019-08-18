#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;
struct dict_index_t;

/** Shift the dense page directory and the array of BLOB pointers when a record
is deleted.
@param[in,out]	page_zip	compressed page
@param[in]	rec		deleted record
@param[in]	index		index of rec
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	free		previous start of the free list */
void page_zip_dir_delete(page_zip_des_t *page_zip, byte *rec,
                         const dict_index_t *index, const ulint *offsets,
                         const byte *free);
