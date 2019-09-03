#include <innodb/trx_trx/trx_arbitrate.h>

#include <innodb/trx_trx/trx_is_autocommit_non_locking.h>
#include <innodb/trx_trx/trx_is_high_priority.h>
#include <innodb/trx_trx/thd_trx_arbitrate.h>
#include <innodb/trx_trx/trx_t.h>


/**
@param[in] requestor	Transaction requesting the lock
@param[in] holder	Transaction holding the lock
@return the transaction that will be rolled back, null don't care */
const trx_t *trx_arbitrate(const trx_t *requestor, const trx_t *holder) {
  ut_ad(!trx_is_autocommit_non_locking(holder));
  ut_ad(!trx_is_autocommit_non_locking(requestor));

  /* Note: Background stats collection transactions also acquire
  locks on user tables. They don't have an associated MySQL session
  instance. */

  if (requestor->mysql_thd == NULL) {
    ut_ad(!trx_is_high_priority(requestor));

    if (trx_is_high_priority(holder)) {
      return (requestor);
    } else {
      return (NULL);
    }

  } else if (holder->mysql_thd == NULL) {
    ut_ad(!trx_is_high_priority(holder));

    if (trx_is_high_priority(requestor)) {
      return (holder);
    }

    return (NULL);
  }

  const THD *victim =
      thd_trx_arbitrate(requestor->mysql_thd, holder->mysql_thd);

  ut_ad(victim == NULL || victim == requestor->mysql_thd ||
        victim == holder->mysql_thd);

  if (victim != NULL) {
    return (victim == requestor->mysql_thd ? requestor : holder);
  }

  return (NULL);
}
