#include <innodb/lock_rec/lock_clust_rec_some_has_impl.h>

#include <innodb/page/page_rec_is_user_rec.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/page/page_rec_is_user_rec.h>

trx_id_t row_get_rec_trx_id(
    const rec_t *rec,          /*!< in: record */
    const dict_index_t *index, /*!< in: clustered index */
    const ulint *offsets);

/** Checks if some transaction has an implicit x-lock on a record in a clustered
 index.
 @return transaction id of the transaction which has the x-lock, or 0 */
trx_id_t lock_clust_rec_some_has_impl(
    const rec_t *rec,          /*!< in: user record */
    const dict_index_t *index, /*!< in: clustered index */
    const ulint *offsets)      /*!< in: rec_get_offsets(rec, index) */
{
  ut_ad(index->is_clustered());
  ut_ad(page_rec_is_user_rec(rec));

  return (row_get_rec_trx_id(rec, index, offsets));
}
