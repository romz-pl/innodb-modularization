#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_rsegf_t.h>

struct mtr_t;

/** Sets the file page number of the nth undo log slot.
@param[in]	rsegf	rollback segment header
@param[in]	n	index of slot
@param[in]	page_no	page number of the undo log segment
@param[in]	mtr	mtr */
void trx_rsegf_set_nth_undo(trx_rsegf_t *rsegf, ulint n, page_no_t page_no,
                            mtr_t *mtr);
