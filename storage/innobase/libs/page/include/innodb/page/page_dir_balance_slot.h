#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Splits a directory slot which owns too many records. */

/** Tries to balance the given directory slot with too few records
 with the upper neighbor, so that there are at least the minimum number
 of records owned by the slot; this may result in the merging of
 two slots. */
void page_dir_balance_slot(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    ulint slot_no);           /*!< in: the directory slot */
