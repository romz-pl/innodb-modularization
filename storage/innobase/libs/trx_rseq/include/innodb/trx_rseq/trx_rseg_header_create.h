#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

struct mtr_t;
class page_size_t;

/** Creates a rollback segment header.
This function is called only when a new rollback segment is created in
the database.
@param[in]	space_id		space id
@param[in]	page_size	page size
@param[in]	max_size	max size in pages
@param[in]	rseg_slot	rseg id == slot number in trx sys
@param[in,out]	mtr		mini-transaction
@return page number of the created segment, FIL_NULL if fail */
page_no_t trx_rseg_header_create(space_id_t space_id,
                                 const page_size_t &page_size,
                                 page_no_t max_size, ulint rseg_slot,
                                 mtr_t *mtr);
