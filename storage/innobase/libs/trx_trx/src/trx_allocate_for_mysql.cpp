#include <innodb/trx_trx/trx_allocate_for_mysql.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_allocate_for_background.h>
#include <innodb/trx_sys/trx_sys_mutex_enter.h>
#include <innodb/trx_sys/trx_sys_mutex_exit.h>

/** Creates a transaction object for MySQL.
 @return own: transaction object */
trx_t *trx_allocate_for_mysql(void) {
  trx_t *trx;

  trx = trx_allocate_for_background();

  trx_sys_mutex_enter();

  ut_d(trx->in_mysql_trx_list = TRUE);
  UT_LIST_ADD_FIRST(trx_sys->mysql_trx_list, trx);

  trx_sys_mutex_exit();

  return (trx);
}
