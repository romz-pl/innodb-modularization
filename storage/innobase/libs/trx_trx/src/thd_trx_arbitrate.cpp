#include <innodb/trx_trx/thd_trx_arbitrate.h>

#include <innodb/assert/assert.h>

#include "include/mysql/plugin.h"

/**
Check if the transaction can be rolled back
@param[in] requestor	Session requesting the lock
@param[in] holder	Session that holds the lock
@return the session that will be rolled back, null don't care */

THD *thd_trx_arbitrate(THD *requestor, THD *holder) {
  /* Non-user (thd==0) transactions by default can't rollback, in
  practice DDL transactions should never rollback and that's because
  they should never wait on table/record locks either */

  ut_a(holder != NULL);
  ut_a(holder != requestor);

  THD *victim = thd_tx_arbitrate(requestor, holder);

  ut_a(victim == NULL || victim == requestor || victim == holder);

  return (victim);
}
