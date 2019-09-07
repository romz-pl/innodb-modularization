#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_rsegf_t.h>

struct mtr_t;

/** Looks for a free slot for an undo log segment.
@param[in]	rsegf	rollback segment header
@param[in]	mtr	mtr
@return slot index or ULINT_UNDEFINED if not found */
ulint trx_rsegf_undo_find_free(trx_rsegf_t *rsegf, mtr_t *mtr);
