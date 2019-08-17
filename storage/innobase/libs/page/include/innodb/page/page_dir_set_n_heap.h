#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Sets the number of records in the heap.
@param[in,out]	page		index page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL. Note that the size of the
                                dense page directory in the compressed page
                                trailer is n_heap * PAGE_ZIP_DIR_SLOT_SIZE.
@param[in]	n_heap		number of records*/
void page_dir_set_n_heap(page_t *page, page_zip_des_t *page_zip, ulint n_heap);
