#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/assert_trx_is_free.h>

/** Check if transaction is in-active so that it can be freed and put back to
transaction pool.
@param t transaction handle */
#define assert_trx_is_inactive(t)              \
  do {                                         \
    assert_trx_is_free((t));                   \
    ut_ad((t)->dict_operation_lock_mode == 0); \
  } while (0)
