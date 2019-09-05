#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/trx_types/trx_state_t.h>
#include <innodb/read/MVCC.h>
#include <innodb/lst/lst.h>

/** Check if transaction is free so that it can be re-initialized.
@param t transaction handle */
#define assert_trx_is_free(t)                            \
  do {                                                   \
    ut_ad(trx_state_eq((t), TRX_STATE_NOT_STARTED) ||    \
          trx_state_eq((t), TRX_STATE_FORCED_ROLLBACK)); \
    ut_ad(!trx_is_rseg_updated(trx));                    \
    ut_ad(!MVCC::is_view_active((t)->read_view));        \
    ut_ad((t)->lock.wait_thr == NULL);                   \
    ut_ad(UT_LIST_GET_LEN((t)->lock.trx_locks) == 0);    \
    ut_ad((t)->dict_operation == TRX_DICT_OP_NONE);      \
  } while (0)
