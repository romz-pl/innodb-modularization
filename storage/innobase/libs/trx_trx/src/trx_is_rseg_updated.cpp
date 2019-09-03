#include <innodb/trx_trx/trx_is_rseg_updated.h>

#include <innodb/trx_trx/trx_is_redo_rseg_updated.h>
#include <innodb/trx_trx/trx_is_temp_rseg_updated.h>

/** Check if redo/noredo rseg is modified for insert/update. */
bool trx_is_rseg_updated(const trx_t *trx) /*!< in: transaction */
{
  return (trx_is_redo_rseg_updated(trx) || trx_is_temp_rseg_updated(trx));
}
