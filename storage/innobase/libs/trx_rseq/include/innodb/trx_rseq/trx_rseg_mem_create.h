#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/purge_pq_t.h>
#include <innodb/univ/page_no_t.h>

class page_size_t;
struct mtr_t;

/** Create and initialize a rollback segment object.  Some of
the values for the fields are read from the segment header page.
The caller must insert it into the correct list.
@param[in]	id		rollback segment id
@param[in]	space_id	space where the segment is placed
@param[in]	page_no		page number of the segment header
@param[in]	page_size	page size
@param[in,out]	purge_queue	rseg queue
@param[in,out]	mtr		mini-transaction
@return own: rollback segment object */
trx_rseg_t *trx_rseg_mem_create(ulint id, space_id_t space_id,
                                page_no_t page_no, const page_size_t &page_size,
                                purge_pq_t *purge_queue, mtr_t *mtr);
