#include <innodb/trx_trx/trx_get_read_view.h>

#include <innodb/read/MVCC.h>
#include <innodb/trx_trx/trx_t.h>

/**
@param trx		Get the active view for this transaction, if one exists
@return the transaction's read view or NULL if one not assigned. */
ReadView *trx_get_read_view(trx_t *trx) {
  return (!MVCC::is_view_active(trx->read_view) ? NULL : trx->read_view);
}


/**
@param trx		Get the active view for this transaction, if one exists
@return the transaction's read view or NULL if one not assigned. */
const ReadView *trx_get_read_view(const trx_t *trx) {
  return (!MVCC::is_view_active(trx->read_view) ? NULL : trx->read_view);
}
