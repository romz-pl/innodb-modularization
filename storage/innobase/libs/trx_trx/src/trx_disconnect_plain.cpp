#include <innodb/trx_trx/trx_disconnect_plain.h>

#include <innodb/trx_trx/trx_disconnect_from_mysql.h>

/** Disconnect a transaction from MySQL.
@param[in,out]	trx	transaction */
void trx_disconnect_plain(trx_t *trx) {
  trx_disconnect_from_mysql(trx, false);
}
