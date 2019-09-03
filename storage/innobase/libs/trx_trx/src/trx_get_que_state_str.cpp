#include <innodb/trx_trx/trx_get_que_state_str.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_types/trx_que_t.h>

/** Retrieves transaction's que state in a human readable string. The string
 should not be free()'d or modified.
 @return string in the data segment */
const char *trx_get_que_state_str(const trx_t *trx) /*!< in: transaction */
{
  /* be sure to adjust TRX_QUE_STATE_STR_MAX_LEN if you change this */
  switch (trx->lock.que_state) {
    case TRX_QUE_RUNNING:
      return ("RUNNING");
    case TRX_QUE_LOCK_WAIT:
      return ("LOCK WAIT");
    case TRX_QUE_ROLLING_BACK:
      return ("ROLLING BACK");
    case TRX_QUE_COMMITTING:
      return ("COMMITTING");
    default:
      return ("UNKNOWN");
  }
}
