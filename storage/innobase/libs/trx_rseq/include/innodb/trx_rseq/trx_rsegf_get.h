#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_rsegf_t.h>

class page_size_t;
struct mtr_t;

/** Gets a rollback segment header.
@param[in]	space		space where placed
@param[in]	page_no		page number of the header
@param[in]	page_size	page size
@param[in,out]	mtr		mini-transaction
@return rollback segment header, page x-latched */
trx_rsegf_t *trx_rsegf_get(space_id_t space, page_no_t page_no,
                           const page_size_t &page_size, mtr_t *mtr);
