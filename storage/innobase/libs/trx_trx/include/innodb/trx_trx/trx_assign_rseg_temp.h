#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Assign a temp-tablespace bound rollback-segment to a transaction.
@param[in,out]	trx	transaction that involves write to temp-table. */
void trx_assign_rseg_temp(trx_t *trx);
