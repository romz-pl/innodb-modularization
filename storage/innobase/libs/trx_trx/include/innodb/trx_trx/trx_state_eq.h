#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_state_t.h>

struct trx_t;

/** Determines if a transaction is in the given state.
 The caller must hold trx_sys->mutex, or it must be the thread
 that is serving a running transaction.
 A running RW transaction must be in trx_sys->rw_trx_list.
 @return true if trx->state == state */
bool trx_state_eq(const trx_t *trx,  /*!< in: transaction */
                  trx_state_t state) /*!< in: state */
    MY_ATTRIBUTE((warn_unused_result));
