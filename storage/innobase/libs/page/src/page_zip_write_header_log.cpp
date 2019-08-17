#include <innodb/page/page_zip_write_header_log.h>

#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_catenate_string.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/header.h>

#ifndef UNIV_HOTBACKUP
/** Write a log record of writing to the uncompressed header portion of a page.
 */
void page_zip_write_header_log(
    const byte *data, /*!< in: data on the uncompressed page */
    ulint length,     /*!< in: length of the data */
    mtr_t *mtr)       /*!< in: mini-transaction */
{
  byte *log_ptr = mlog_open(mtr, 11 + 1 + 1);
  ulint offset = page_offset(data);

  ut_ad(offset < PAGE_DATA);
  ut_ad(offset + length < PAGE_DATA);

  static_assert(PAGE_DATA <= 255, "PAGE_DATA > 255");

  ut_ad(length < 256);

  /* If no logging is requested, we may return now */
  if (UNIV_UNLIKELY(!log_ptr)) {
    return;
  }

  log_ptr = mlog_write_initial_log_record_fast(
      (byte *)data, MLOG_ZIP_WRITE_HEADER, log_ptr, mtr);
  *log_ptr++ = (byte)offset;
  *log_ptr++ = (byte)length;
  mlog_close(mtr, log_ptr);

  mlog_catenate_string(mtr, data, length);
}
#endif /* !UNIV_HOTBACKUP */
