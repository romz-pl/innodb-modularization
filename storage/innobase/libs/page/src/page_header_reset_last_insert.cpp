#include <innodb/page/page_header_reset_last_insert.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/header.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/page/page_zip_write_header.h>
#include <innodb/mtr/mlog_write_ulint.h>


/** Resets the last insert info field in the page header. Writes to mlog
 about this operation. */
void page_header_reset_last_insert(
    page_t *page,             /*!< in/out: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be updated, or NULL */
    mtr_t *mtr)               /*!< in: mtr */
{
  ut_ad(page != NULL);
  ut_ad(mtr != NULL);

  if (page_zip) {
    mach_write_to_2(page + (PAGE_HEADER + PAGE_LAST_INSERT), 0);
    page_zip_write_header(page_zip, page + (PAGE_HEADER + PAGE_LAST_INSERT), 2,
                          mtr);
  } else {
    mlog_write_ulint(page + (PAGE_HEADER + PAGE_LAST_INSERT), 0, MLOG_2BYTES,
                     mtr);
  }
}
#endif /* !UNIV_HOTBACKUP */

