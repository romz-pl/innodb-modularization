#include <innodb/trx_trx/thd_trx_priority.h>

#include "include/mysql/plugin.h"

int thd_trx_priority(THD *thd) {
  return (thd == NULL ? 0 : thd_tx_priority(thd));
}
