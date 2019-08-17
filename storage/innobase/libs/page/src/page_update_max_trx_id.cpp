#include <innodb/page/page_update_max_trx_id.h>


#include <innodb/mtr/mtr_memo_contains.h>
#include <innodb/log_types/recv_recovery_is_on.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/buf_block/buf_block_get_frame.h>
#include <innodb/page/page_get_max_trx_id.h>
#include <innodb/page/page_set_max_trx_id.h>


/** Sets the max trx id field value if trx_id is bigger than the previous
 value. */
void page_update_max_trx_id(
    buf_block_t *block,       /*!< in/out: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be updated, or NULL */
    trx_id_t trx_id,          /*!< in: transaction id */
    mtr_t *mtr)               /*!< in/out: mini-transaction */
{
  ut_ad(block);
#ifndef UNIV_HOTBACKUP
  ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
#endif /* !UNIV_HOTBACKUP */
  /* During crash recovery, this function may be called on
  something else than a leaf page of a secondary index or the
  insert buffer index tree (dict_index_is_sec_or_ibuf() returns
  TRUE for the dummy indexes constructed during redo log
  application).  In that case, PAGE_MAX_TRX_ID is unused,
  and trx_id is usually zero. */
  ut_ad(trx_id || recv_recovery_is_on());
  ut_ad(page_is_leaf(buf_block_get_frame(block)));

  if (page_get_max_trx_id(buf_block_get_frame(block)) < trx_id) {
    page_set_max_trx_id(block, page_zip, trx_id, mtr);
  }
}

