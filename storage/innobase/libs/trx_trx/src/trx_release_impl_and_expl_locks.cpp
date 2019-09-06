#include <innodb/trx_trx/trx_release_impl_and_expl_locks.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/check_trx_state.h>
#include <innodb/trx_trx/trx_erase_lists.h>
#include <innodb/trx_sys/trx_sys.h>
#include <innodb/trx_sys/trx_sys_mutex_exit.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/trx_sys/trx_sys_mutex_enter.h>
#include <innodb/trx_trx/trx_mutex_enter.h>
#include <innodb/trx_trx/trx_mutex_exit.h>


void lock_trx_release_locks(trx_t *trx);

/* The following function makes the transaction committed in memory
and makes its changes to data visible to other transactions.
In particular it releases implicit and explicit locks held by transaction and
transitions to the transaction to the TRX_STATE_COMMITTED_IN_MEMORY state.
NOTE that there is a small discrepancy from the strict formal
visibility rules here: a human user of the database can see
modifications made by another transaction T even before the necessary
log segment has been flushed to the disk. If the database happens to
crash before the flush, the user has seen modifications from T which
will never be a committed transaction. However, any transaction T2
which sees the modifications of the committing transaction T, and
which also itself makes modifications to the database, will get an lsn
larger than the committing transaction T. In the case where the log
flush fails, and T never gets committed, also T2 will never get
committed.
@param[in,out]  trx         The transaction for which will be committed in
                            memory
@param[in]      serialized  true if serialisation log was written. Affects the
                            list of things we need to clean up during
                            trx_erase_lists.
*/
void trx_release_impl_and_expl_locks(trx_t *trx, bool serialized) {
  check_trx_state(trx);
  ut_ad(trx_state_eq(trx, TRX_STATE_ACTIVE) ||
        trx_state_eq(trx, TRX_STATE_PREPARED));

  bool trx_sys_latch_is_needed =
      (trx->id > 0) || trx_state_eq(trx, TRX_STATE_PREPARED);

  if (trx_sys_latch_is_needed) {
    trx_sys_mutex_enter();
  }

  if (trx->id > 0) {
    /* For consistent snapshot, we need to remove current
    transaction from running transaction id list for mvcc
    before doing commit and releasing locks. */
    trx_erase_lists(trx, serialized);
  }

  if (trx_state_eq(trx, TRX_STATE_PREPARED)) {
    ut_a(trx_sys->n_prepared_trx > 0);
    --trx_sys->n_prepared_trx;
  }

  trx_mutex_enter(trx);
  /* Please consider this particular point in time as the moment the trx's
  implicit locks become released.
  This change is protected by both trx_sys->mutex and trx->mutex.
  Therefore, there are two secure ways to check if the trx still can hold
  implicit locks:
  (1) if you only know id of the trx, then you can obtain trx_sys->mutex and
      check if trx is still in rw_trx_set. This works, because the call to
      trx_erase_list() which removes trx from this list several lines above is
      also protected by trx_sys->mutex. We use this approach in
      lock_rec_convert_impl_to_expl() by using trx_rw_is_active()
  (2) if you have pointer to trx, and you know it is safe to access (say, you
      hold reference to this trx which prevents it from being freed) then you
      can obtain trx->mutex and check if trx->state is equal to
      TRX_STATE_COMMITTED_IN_MEMORY. We use this approach in
      lock_rec_convert_impl_to_expl_for_trx() when deciding for the final time
      if we really want to create explicit lock on behalf of implicit lock
      holder. */
  trx->state = TRX_STATE_COMMITTED_IN_MEMORY;
  trx_mutex_exit(trx);

  if (trx_sys_latch_is_needed) {
    trx_sys_mutex_exit();
  }

  lock_trx_release_locks(trx);
}
