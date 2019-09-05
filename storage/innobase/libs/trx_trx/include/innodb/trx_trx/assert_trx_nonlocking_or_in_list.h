#pragma once

#include <innodb/univ/univ.h>

struct trx_t;


#ifdef UNIV_DEBUG
/** Assert that an autocommit non-locking select cannot be in the
 rw_trx_list and that it is a read-only transaction.
 The tranasction must be in the mysql_trx_list. */
#define assert_trx_nonlocking_or_in_list(t)         \
  do {                                              \
    if (trx_is_autocommit_non_locking(t)) {         \
      trx_state_t t_state = (t)->state;             \
      ut_ad((t)->read_only);                        \
      ut_ad(!(t)->is_recovered);                    \
      ut_ad(!(t)->in_rw_trx_list);                  \
      ut_ad((t)->in_mysql_trx_list);                \
      ut_ad(t_state == TRX_STATE_NOT_STARTED ||     \
            t_state == TRX_STATE_FORCED_ROLLBACK || \
            t_state == TRX_STATE_ACTIVE);           \
    } else {                                        \
      check_trx_state(t);                           \
    }                                               \
  } while (0)
#else /* UNIV_DEBUG */
/** Assert that an autocommit non-locking slect cannot be in the
 rw_trx_list and that it is a read-only transaction.
 The tranasction must be in the mysql_trx_list. */
#define assert_trx_nonlocking_or_in_list(trx) ((void)0)
#endif /* UNIV_DEBUG */
