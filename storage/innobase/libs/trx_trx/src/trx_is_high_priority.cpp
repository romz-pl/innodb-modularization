#include <innodb/trx_trx/trx_is_high_priority.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/thd_trx_priority.h>

/**
@param[in] trx		Transaction to check
@return true if the transaction is a high priority transaction.*/
bool trx_is_high_priority(const trx_t *trx) {
  if (trx->mysql_thd == NULL) {
    return (false);
  }

  return (thd_trx_priority(trx->mysql_thd) > 0);
}
