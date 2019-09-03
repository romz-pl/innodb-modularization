#include <innodb/trx_trx/trx_is_redo_rseg_updated.h>

#include <innodb/trx_trx/trx_t.h>

/** Check if redo rseg is modified for insert/update. */
bool trx_is_redo_rseg_updated(const trx_t *trx) /*!< in: transaction */
{
  return (trx->rsegs.m_redo.insert_undo != 0 ||
          trx->rsegs.m_redo.update_undo != 0);
}
