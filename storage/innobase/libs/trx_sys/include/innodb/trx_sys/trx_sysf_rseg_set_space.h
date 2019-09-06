#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_sysf_t.h>

struct mtr_t;

/** Sets the space id of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	space		space id
@param[in]	mtr		mtr */
void trx_sysf_rseg_set_space(trx_sysf_t *sys_header, ulint i, space_id_t space,
                             mtr_t *mtr);
