#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>

/**
Determine if the transaction is a non-locking autocommit select
(implied read-only).
@param t transaction
@return true if non-locking autocommit select transaction. */
#define trx_is_autocommit_non_locking(t) \
  ((t)->auto_commit && (t)->will_lock == 0)
