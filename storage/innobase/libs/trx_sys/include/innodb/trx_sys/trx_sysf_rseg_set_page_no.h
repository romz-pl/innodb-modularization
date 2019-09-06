#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_sysf_t.h>
#include <innodb/univ/page_no_t.h>

struct mtr_t;

/** Set the page number of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	page_no		page number, FIL_NULL if the slot is reset to
                                unused
@param[in]	mtr		mtr */
void trx_sysf_rseg_set_page_no(trx_sysf_t *sys_header, ulint i,
                               page_no_t page_no, mtr_t *mtr);
