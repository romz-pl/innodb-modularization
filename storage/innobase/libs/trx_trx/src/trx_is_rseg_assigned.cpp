#include <innodb/trx_trx/trx_is_rseg_assigned.h>

#include <innodb/trx_trx/trx_t.h>

/** Check if redo/nonredo rseg is valid. */
bool trx_is_rseg_assigned(const trx_t *trx) /*!< in: transaction */
{
  return (trx->rsegs.m_redo.rseg != NULL || trx->rsegs.m_noredo.rseg != NULL);
}
