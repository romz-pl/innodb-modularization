#include <innodb/tablespace/fsp_is_inode_page.h>

#include <innodb/tablespace/consts.h>
#include <innodb/assert/assert.h>
#include <innodb/page/univ_page_size.h>

/** Check if a specified page is inode page or not. This is used for
index root pages of hard-coded DD tables, we can safely assume that the passed
in page number is in the range of pages which are only either index root page
or inode page
@param[in]	page	Page number to check
@return true if it's inode page, otherwise false */
bool fsp_is_inode_page(page_no_t page) {
  static const uint inode_per_page = FSP_SEG_INODES_PER_PAGE(univ_page_size);

  /* Every two inode would be allocated for one index, and all inodes in
  two inode pages are needed exactly for FSP_SEG_INODES_PER_PAGE indexes.
  One inode page is at the beginning of one cycle, the other is in the
  middle. If FSP_SEG_INODES_PER_PAGE is even, the second inode page
  is at cycle / 2, if odd, it should be (cycle + 1) / 2 in the cycle. */
  static const uint cycle = inode_per_page + 2;

  /* Number of all hard-coded DD table indexes. Please sync it with
  innodb_dd_table array. */
  static const uint indexes = 98;

  /* Max page number for index root pages of hard-coded DD tables. */
  static const uint max_page_no =
      FSP_FIRST_INODE_PAGE_NO + 1 /* SDI Index page */
      + (indexes / inode_per_page) * cycle + (indexes % inode_per_page) +
      ((indexes % inode_per_page) / ((inode_per_page + 1) / 2));

  /* The page range should be determinate for different page sizes. */
  ut_a(page >= FSP_FIRST_INODE_PAGE_NO);
  ut_a(page <= max_page_no);

  uint step = (page - FSP_FIRST_INODE_PAGE_NO) % cycle;

  return (step == 0 || step == (cycle + 1) / 2);
}

