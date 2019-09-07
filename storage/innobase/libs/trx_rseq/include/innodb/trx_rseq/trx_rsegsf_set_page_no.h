#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_rsegsf_t.h>

struct mtr_t;

/** Sets the page number of the nth rollback segment slot in the
independent undo tablespace.
@param[in]	rsegs_header	rollback segment array page header
@param[in]	slot		slot number on page  == rseg id
@param[in]	page_no		rollback regment header page number
@param[in]	mtr		mtr */
void trx_rsegsf_set_page_no(trx_rsegsf_t *rsegs_header, ulint slot,
                            page_no_t page_no, mtr_t *mtr);
