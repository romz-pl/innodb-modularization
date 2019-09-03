#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_is_autocommit_non_locking.h>

/**
Determine if the transaction is a non-locking autocommit select
with an explicit check for the read-only status.
@param t transaction
@return true if non-locking autocommit read-only transaction. */
#define trx_is_ac_nl_ro(t) \
  ((t)->read_only && trx_is_autocommit_non_locking((t)))
