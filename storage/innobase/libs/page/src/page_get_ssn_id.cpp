#include <innodb/page/page_get_ssn_id.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/disk/flags.h>

/** Returns the RTREE SPLIT SEQUENCE NUMBER (FIL_RTREE_SPLIT_SEQ_NUM).
@param[in]	page	page
@return SPLIT SEQUENCE NUMBER */
node_seq_t page_get_ssn_id(const page_t *page) {
  ut_ad(page);

  return (static_cast<node_seq_t>(
      mach_read_from_8(page + FIL_RTREE_SPLIT_SEQ_NUM)));
}
