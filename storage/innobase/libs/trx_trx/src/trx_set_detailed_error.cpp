#include <innodb/trx_trx/trx_set_detailed_error.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/flags.h>
#include <innodb/string/ut_strlcpy.h>

/** Set detailed error message for the transaction. */
void trx_set_detailed_error(trx_t *trx,      /*!< in: transaction struct */
                            const char *msg) /*!< in: detailed error message */
{
  ut_strlcpy(trx->detailed_error, msg, MAX_DETAILED_ERROR_LEN);
}
