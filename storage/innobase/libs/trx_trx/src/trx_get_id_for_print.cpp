#include <innodb/trx_trx/trx_get_id_for_print.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/data_types/flags.h>

/** Retreieves the transaction ID.
In a given point in time it is guaranteed that IDs of the running
transactions are unique. The values returned by this function for readonly
transactions may be reused, so a subsequent RO transaction may get the same ID
as a RO transaction that existed in the past. The values returned by this
function should be used for printing purposes only.
@param[in]	trx	transaction whose id to retrieve
@return transaction id */
trx_id_t trx_get_id_for_print(const trx_t *trx) {
  /* Readonly and transactions whose intentions are unknown (whether
  they will eventually do a WRITE) don't have trx_t::id assigned (it is
  0 for those transactions). Transaction IDs in
  information_schema.innodb_trx.trx_id,
  performance_schema.data_locks.engine_transaction_id,
  performance_schema.data_lock_waits.requesting_engine_transaction_id,
  performance_schema.data_lock_waits.blocking_engine_transaction_id
  should match because those tables
  could be used in an SQL JOIN on those columns. Also trx_t::id is
  printed by SHOW ENGINE INNODB STATUS, and in logs, so we must have the
  same value printed everywhere consistently. */

  /* DATA_TRX_ID_LEN is the storage size in bytes. */
  static const trx_id_t max_trx_id = (1ULL << (DATA_TRX_ID_LEN * CHAR_BIT)) - 1;

  ut_ad(trx->id <= max_trx_id);

  return (trx->id != 0 ? trx->id
                       : reinterpret_cast<trx_id_t>(trx) | (max_trx_id + 1));
}
