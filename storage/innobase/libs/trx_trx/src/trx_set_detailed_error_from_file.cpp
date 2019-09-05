#include <innodb/trx_trx/trx_set_detailed_error_from_file.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/flags.h>
#include <innodb/io/os_file_read_string.h>

/** Set detailed error message for the transaction from a file. Note that the
 file is rewinded before reading from it. */
void trx_set_detailed_error_from_file(
    trx_t *trx, /*!< in: transaction struct */
    FILE *file) /*!< in: file to read message from */
{
  os_file_read_string(file, trx->detailed_error, MAX_DETAILED_ERROR_LEN);
}
