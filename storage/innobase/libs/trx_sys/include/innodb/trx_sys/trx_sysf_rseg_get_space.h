#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_sysf_t.h>

struct mtr_t;

/** Gets the space of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	mtr		mtr
@return space id */
space_id_t trx_sysf_rseg_get_space(trx_sysf_t *sys_header, ulint i, mtr_t *mtr);
