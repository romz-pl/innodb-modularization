#include <innodb/trx_sys/trx_assert_recovered.h>


#if defined UNIV_DEBUG || defined UNIV_BLOB_LIGHT_DEBUG
/** Assert that a transaction has been recovered.
 @return true */
UNIV_INLINE
ibool trx_assert_recovered(trx_id_t trx_id) /*!< in: transaction identifier */
{
  const trx_t *trx;

  trx_sys_mutex_enter();

  trx = trx_get_rw_trx_by_id(trx_id);
  ut_a(trx->is_recovered);

  trx_sys_mutex_exit();

  return (TRUE);
}
#endif /* UNIV_DEBUG || UNIV_BLOB_LIGHT_DEBUG */
