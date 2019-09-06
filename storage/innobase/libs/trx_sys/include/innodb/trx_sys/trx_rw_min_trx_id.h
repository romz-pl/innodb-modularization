#pragma once

#include <innodb/univ/univ.h>

/** Returns the minimum trx id in rw trx list. This is the smallest id for which
 the trx can possibly be active. (But, you must look at the trx->state to
 find out if the minimum trx id transaction itself is active, or already
 committed.)
 @return the minimum trx id, or trx_sys->max_trx_id if the trx list is empty */
trx_id_t trx_rw_min_trx_id(void);
