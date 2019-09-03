#include <innodb/trx_sys/trx_sys_get_max_trx_id.h>

#include <innodb/trx_sys/trx_sys.h>
#include <innodb/trx_sys/trx_sys_mutex_own.h>
#include <innodb/trx_sys/trx_sys_mutex_exit.h>
#include <innodb/data_types/flags.h>
#include <innodb/trx_sys/trx_sys_mutex_enter.h>

trx_id_t trx_sys_get_max_trx_id() {
  ut_ad(!trx_sys_mutex_own());

  if (UNIV_WORD_SIZE < DATA_TRX_ID_LEN) {
    /* Avoid torn reads. */

    trx_sys_mutex_enter();

    trx_id_t max_trx_id = trx_sys->max_trx_id;

    trx_sys_mutex_exit();

    return (max_trx_id);
  } else {
    /* Perform a dirty read. Callers should be prepared for stale
    values, and we know that the value fits in a machine word, so
    that it will be read and written atomically. */

    return (trx_sys->max_trx_id);
  }
}
