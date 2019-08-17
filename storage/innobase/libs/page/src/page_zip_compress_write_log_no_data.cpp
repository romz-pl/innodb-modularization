#include <innodb/page/page_zip_compress_write_log_no_data.h>

#include <innodb/mtr/mlog_open_and_write_index.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/machine/data.h>

/** Write a log record of compressing an index page without the data on the
 * page.
 */
void page_zip_compress_write_log_no_data(
    ulint level,         /*!< in: compression level */
    const page_t *page,  /*!< in: page that is compressed */
    dict_index_t *index, /*!< in: index */
    mtr_t *mtr)          /*!< in: mtr */
{
#ifndef UNIV_HOTBACKUP
  byte *log_ptr = mlog_open_and_write_index(mtr, page, index,
                                            MLOG_ZIP_PAGE_COMPRESS_NO_DATA, 1);

  if (log_ptr) {
    mach_write_to_1(log_ptr, level);
    mlog_close(mtr, log_ptr + 1);
  }
#endif /* !UNIV_HOTBACKUP */
}
