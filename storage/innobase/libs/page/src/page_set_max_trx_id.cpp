#include <innodb/page/page_set_max_trx_id.h>

#include <innodb/buf_block/buf_block_get_frame.h>
#include <innodb/mtr/mtr_memo_contains.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_zip_write_header.h>
#include <innodb/mtr/mlog_write_ull.h>
#include <innodb/page/header.h>

/** Sets the max trx id field value. */
void page_set_max_trx_id(
    buf_block_t *block,       /*!< in/out: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    trx_id_t trx_id,          /*!< in: transaction id */
    mtr_t *mtr)               /*!< in/out: mini-transaction, or NULL */
{
  page_t *page = buf_block_get_frame(block);
#ifndef UNIV_HOTBACKUP
  ut_ad(!mtr || mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
#endif /* !UNIV_HOTBACKUP */

  /* It is not necessary to write this change to the redo log, as
  during a database recovery we assume that the max trx id of every
  page is the maximum trx id assigned before the crash. */

  if (page_zip) {
    mach_write_to_8(page + (PAGE_HEADER + PAGE_MAX_TRX_ID), trx_id);
    page_zip_write_header(page_zip, page + (PAGE_HEADER + PAGE_MAX_TRX_ID), 8,
                          mtr);
#ifndef UNIV_HOTBACKUP
  } else if (mtr) {
    mlog_write_ull(page + (PAGE_HEADER + PAGE_MAX_TRX_ID), trx_id, mtr);
#endif /* !UNIV_HOTBACKUP */
  } else {
    mach_write_to_8(page + (PAGE_HEADER + PAGE_MAX_TRX_ID), trx_id);
  }
}
