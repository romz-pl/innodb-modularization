#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Sets the number of dir slots in directory.
@param[in,out]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	n_slots		number of slots */
void page_dir_set_n_slots(page_t *page, page_zip_des_t *page_zip,
                          ulint n_slots);
