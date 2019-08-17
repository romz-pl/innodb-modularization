#include <innodb/page/page_set_ssn_id.h>

#include <innodb/assert/assert.h>
#include <innodb/buf_block/buf_block_get_frame.h>
#include <innodb/disk/page_t.h>
#include <innodb/machine/data.h>
#include <innodb/mtr/mlog_write_ull.h>
#include <innodb/mtr/mtr_memo_contains.h>
#include <innodb/page/page_zip_write_header.h>


/** Sets the RTREE SPLIT SEQUENCE NUMBER field value
@param[in,out]	block		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	ssn_id		transaction id
@param[in,out]	mtr		mini-transaction */
void page_set_ssn_id(buf_block_t *block, page_zip_des_t *page_zip,
                     node_seq_t ssn_id, mtr_t *mtr) {
  page_t *page = buf_block_get_frame(block);
#ifndef UNIV_HOTBACKUP
  ut_ad(!mtr || mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_SX_FIX) ||
        mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
#endif /* !UNIV_HOTBACKUP */

  if (page_zip) {
    mach_write_to_8(page + FIL_RTREE_SPLIT_SEQ_NUM, ssn_id);
    page_zip_write_header(page_zip, page + FIL_RTREE_SPLIT_SEQ_NUM, 8, mtr);
#ifndef UNIV_HOTBACKUP
  } else if (mtr) {
    mlog_write_ull(page + FIL_RTREE_SPLIT_SEQ_NUM, ssn_id, mtr);
#endif /* !UNIV_HOTBACKUP */
  } else {
    mach_write_to_8(page + FIL_RTREE_SPLIT_SEQ_NUM, ssn_id);
  }
}
