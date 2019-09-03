#include <innodb/trx_trx/trx_get_error_index.h>

#include <innodb/trx_trx/trx_t.h>

/** Retrieves the index causing error from a trx.
@param[in]	trx	trx object
@return the error index */
const dict_index_t *trx_get_error_index(const trx_t *trx) {
  return (trx->error_index);
}
