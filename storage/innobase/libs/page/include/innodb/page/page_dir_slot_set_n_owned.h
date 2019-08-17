#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_dir_slot_t.h>

struct page_zip_des_t;

/** This is used to set the owned records field of a directory slot.
@param[in,out]	slot		directory slot
@param[in,out]	page_zip	compressed page, or NULL
@param[in]	n		number of records owned by the slot */
void page_dir_slot_set_n_owned(page_dir_slot_t *slot, page_zip_des_t *page_zip,
                               ulint n);
