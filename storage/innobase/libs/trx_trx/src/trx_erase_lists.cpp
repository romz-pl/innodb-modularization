#include <innodb/trx_trx/trx_erase_lists.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_sys/trx_sys.h>
#include <innodb/read/MVCC.h>

/**
Erase the transaction from running transaction lists and serialization
list. Active RW transaction list of a MVCC snapshot(ReadView::prepare)
won't include this transaction after this call. All implicit locks are
also released by this call as trx is removed from rw_trx_list.
@param[in] trx		Transaction to erase, must have an ID > 0
@param[in] serialised	true if serialisation log was written */
void trx_erase_lists(trx_t *trx, bool serialised) {
  ut_ad(trx->id > 0);
  ut_ad(trx_sys_mutex_own());

  if (serialised) {
    UT_LIST_REMOVE(trx_sys->serialisation_list, trx);
  }

  trx_ids_t::iterator it = std::lower_bound(trx_sys->rw_trx_ids.begin(),
                                            trx_sys->rw_trx_ids.end(), trx->id);
  ut_ad(*it == trx->id);
  trx_sys->rw_trx_ids.erase(it);

  if (trx->read_only || trx->rsegs.m_redo.rseg == NULL) {
    ut_ad(!trx->in_rw_trx_list);
  } else {
    UT_LIST_REMOVE(trx_sys->rw_trx_list, trx);
    ut_d(trx->in_rw_trx_list = false);
    ut_ad(trx_sys_validate_trx_list());

    if (trx->read_view != NULL) {
      trx_sys->mvcc->view_close(trx->read_view, true);
    }
  }

  trx_sys->rw_trx_set.erase(TrxTrack(trx->id));

  /* Set minimal active trx id. */
  trx_id_t min_id = trx_sys->rw_trx_ids.empty() ? trx_sys->max_trx_id
                                                : trx_sys->rw_trx_ids.front();

  trx_sys->min_active_id.store(min_id);
}
