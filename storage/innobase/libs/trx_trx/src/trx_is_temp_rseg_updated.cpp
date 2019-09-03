#include <innodb/trx_trx/trx_is_temp_rseg_updated.h>

#include <innodb/trx_trx/trx_t.h>

/** Check if noredo rseg is modified for insert/update. */
bool trx_is_temp_rseg_updated(const trx_t *trx) /*!< in: transaction */
{
  return (trx->rsegs.m_noredo.insert_undo != 0 ||
          trx->rsegs.m_noredo.update_undo != 0);
}
