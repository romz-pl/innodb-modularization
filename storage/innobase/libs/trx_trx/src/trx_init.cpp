#include <innodb/trx_trx/trx_init.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_types/flags.h>
#include <innodb/trx_trx/trx_isolation_level.h>
#include <innodb/trx_trx/TrxInInnoDB.h>

/** Initialize transaction object.
 @param trx trx to initialize */
void trx_init(trx_t *trx) {
  /* This is called at the end of commit, do not reset the
  trx_t::state here to NOT_STARTED. The FORCED_ROLLBACK
  status is required for asynchronous handling. */

  trx->id = 0;

  trx->no = TRX_ID_MAX;

  trx->skip_lock_inheritance = false;

  trx->is_recovered = false;

  trx->op_info = "";

  trx->isolation_level = TRX_ISO_REPEATABLE_READ;

  trx->check_foreigns = true;

  trx->check_unique_secondary = true;

  trx->lock.n_rec_locks = 0;

  trx->dict_operation = TRX_DICT_OP_NONE;

  trx->ddl_operation = false;

  trx->error_state = DB_SUCCESS;

  trx->error_key_num = ULINT_UNDEFINED;

  trx->undo_no = 0;

  trx->rsegs.m_redo.rseg = NULL;

  trx->rsegs.m_noredo.rseg = NULL;

  trx->read_only = false;

  trx->auto_commit = false;

  trx->will_lock = 0;

  trx->internal = false;

  trx->in_truncate = false;
#ifdef UNIV_DEBUG
  trx->is_dd_trx = false;
  trx->in_rollback = false;
  trx->lock.in_rollback = false;
#endif /* UNIV_DEBUG */

  ut_d(trx->start_file = 0);

  ut_d(trx->start_line = 0);

  trx->magic_n = TRX_MAGIC_N;

  trx->lock.que_state = TRX_QUE_RUNNING;

  trx->last_sql_stat_start.least_undo_no = 0;

  ut_ad(!MVCC::is_view_active(trx->read_view));

  trx->lock.rec_cached = 0;

  trx->lock.table_cached = 0;

  trx->error_index = nullptr;

  /* During asynchronous rollback, we should reset forced rollback flag
  only after rollback is complete to avoid race with the thread owning
  the transaction. */

  if (!TrxInInnoDB::is_async_rollback(trx)) {
    os_thread_id_t thread_id = trx->killed_by;
    os_compare_and_swap_thread_id(&trx->killed_by, thread_id, 0);

    /* Note: Do not set to 0, the ref count is decremented inside
    the TrxInInnoDB() destructor. We only need to clear the flags. */

    trx->in_innodb &= TRX_FORCE_ROLLBACK_MASK;
  }

  /* Note: It's possible that this list is not empty if a transaction
  was interrupted after it collected the victim transactions and before
  it got a chance to roll them back asynchronously. */

  trx->hit_list.clear();

  trx->flush_observer = NULL;

  ++trx->version;
}
