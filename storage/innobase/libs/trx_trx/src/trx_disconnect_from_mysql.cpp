#include <innodb/trx_trx/trx_disconnect_from_mysql.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_sys/trx_sys_mutex_exit.h>
#include <innodb/trx_sys/trx_sys_mutex_enter.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/read/MVCC.h>


/** Disconnect a transaction from MySQL and optionally mark it as if
it's been recovered. For the marking the transaction must be in prepared state.
The recovery-marked transaction is going to survive "alone" so its association
with the mysql handle is destroyed now rather than when it will be
finally freed.
@param[in,out]	trx		transaction
@param[in]	prepared	boolean value to specify whether trx is
                                for recovery or not. */
void trx_disconnect_from_mysql(trx_t *trx, bool prepared) {
  trx_sys_mutex_enter();

  ut_ad(trx->in_mysql_trx_list);
  ut_d(trx->in_mysql_trx_list = FALSE);

  UT_LIST_REMOVE(trx_sys->mysql_trx_list, trx);

  if (trx->read_view != NULL) {
    trx_sys->mvcc->view_close(trx->read_view, true);
  }

  ut_ad(trx_sys_validate_trx_list());

  if (prepared) {
    ut_ad(trx_state_eq(trx, TRX_STATE_PREPARED));

    trx->is_recovered = true;
    trx->mysql_thd = NULL;
    /* todo/fixme: suggest to do it at innodb prepare */
    trx->will_lock = 0;
  }

  trx_sys_mutex_exit();
}
