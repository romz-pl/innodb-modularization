#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_sysf_t.h>
#include <innodb/univ/page_no_t.h>

struct mtr_t;

/** Gets the page number of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	mtr		mtr
@return page number, FIL_NULL if slot unused */
page_no_t trx_sysf_rseg_get_page_no(trx_sysf_t *sys_header, ulint i,
                                    mtr_t *mtr);
