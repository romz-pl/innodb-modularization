#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_rsegf_t.h>

struct mtr_t;

/** Gets the file page number of the nth undo log slot.
@param[in]	rsegf	rollback segment header
@param[in]	n	index of slot
@param[in]	mtr	mtr
@return page number of the undo log segment */
page_no_t trx_rsegf_get_nth_undo(trx_rsegf_t *rsegf, ulint n, mtr_t *mtr);
