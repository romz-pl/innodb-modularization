#include <innodb/trx_trx/trx_free_for_mysql.h>

#include <innodb/trx_trx/trx_disconnect_plain.h>
#include <innodb/trx_trx/trx_free_for_background.h>

/** Free a transaction object for MySQL.
@param[in,out]	trx	transaction */
void trx_free_for_mysql(trx_t *trx) {
  trx_disconnect_plain(trx);
  trx_free_for_background(trx);
}
