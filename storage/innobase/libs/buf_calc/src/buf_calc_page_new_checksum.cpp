#include <innodb/buf_calc/buf_calc_page_new_checksum.h>

#include <innodb/random/random.h>
#include <innodb/disk/flags.h>

/** Calculates a page checksum which is stored to the page when it is written
 to a file. Note that we must be careful to calculate the same value on
 32-bit and 64-bit architectures.
 @return checksum */
ulint buf_calc_page_new_checksum(const byte *page) /*!< in: buffer page */
{
  ulint checksum;

  /* Since the field FIL_PAGE_FILE_FLUSH_LSN, and in versions <= 4.1.x
  FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID, are written outside the buffer pool
  to the first pages of data files, we have to skip them in the page
  checksum calculation.
  We must also skip the field FIL_PAGE_SPACE_OR_CHKSUM where the
  checksum is stored, and also the last 8 bytes of page because
  there we store the old formula checksum. */

  checksum =
      ut_fold_binary(page + FIL_PAGE_OFFSET,
                     FIL_PAGE_FILE_FLUSH_LSN - FIL_PAGE_OFFSET) +
      ut_fold_binary(page + FIL_PAGE_DATA, UNIV_PAGE_SIZE - FIL_PAGE_DATA -
                                               FIL_PAGE_END_LSN_OLD_CHKSUM);
  checksum = checksum & 0xFFFFFFFFUL;

  return (checksum);
}
