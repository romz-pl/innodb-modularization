#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

class page_id_t;
class page_size_t;
struct mtr_t;

/** Gets an undo log page and x-latches it.
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in,out]	mtr		mini-transaction
@return pointer to page x-latched */
page_t *trx_undo_page_get(const page_id_t &page_id,
                          const page_size_t &page_size, mtr_t *mtr);
