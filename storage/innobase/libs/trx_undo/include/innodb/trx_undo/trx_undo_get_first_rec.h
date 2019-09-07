#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_undo_rec_t.h>

class page_size_t;
struct mtr_t;

/** Gets the first record in an undo log.
@param[out]	modifier_trx_id	the modifier trx identifier.
@param[in]	space		undo log header space
@param[in]	page_size	page size
@param[in]	page_no		undo log header page number
@param[in]	offset		undo log header offset on page
@param[in]	mode		latching mode: RW_S_LATCH or RW_X_LATCH
@param[in,out]	mtr		mini-transaction
@return undo log record, the page latched, NULL if none */
trx_undo_rec_t *trx_undo_get_first_rec(trx_id_t *modifier_trx_id,
                                       space_id_t space,
                                       const page_size_t &page_size,
                                       page_no_t page_no, ulint offset,
                                       ulint mode, mtr_t *mtr);
