#include <innodb/trx_trx/trx_disconnect_prepared.h>

#include <innodb/trx_trx/trx_disconnect_from_mysql.h>

/** Disconnect a prepared transaction from MySQL.
@param[in,out]	trx	transaction */
void trx_disconnect_prepared(trx_t *trx) {
  trx_disconnect_from_mysql(trx, true);
}
