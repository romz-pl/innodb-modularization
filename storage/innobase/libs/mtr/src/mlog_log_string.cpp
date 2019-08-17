#include <innodb/mtr/mlog_log_string.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_catenate_string.h>
#include <innodb/page/page_offset.h>


/** Logs a write of a string to a file page buffered in the buffer pool.
 Writes the corresponding log record to the mini-transaction log. */
void mlog_log_string(byte *ptr,  /*!< in: pointer written to */
                     ulint len,  /*!< in: string length */
                     mtr_t *mtr) /*!< in: mini-transaction handle */
{
  byte *log_ptr;

  ut_ad(ptr && mtr);
  ut_ad(len <= UNIV_PAGE_SIZE);

  log_ptr = mlog_open(mtr, 30);

  /* If no logging is requested, we may return now */
  if (log_ptr == NULL) {
    return;
  }

  log_ptr =
      mlog_write_initial_log_record_fast(ptr, MLOG_WRITE_STRING, log_ptr, mtr);
  mach_write_to_2(log_ptr, page_offset(ptr));
  log_ptr += 2;

  mach_write_to_2(log_ptr, len);
  log_ptr += 2;

  mlog_close(mtr, log_ptr);

  mlog_catenate_string(mtr, ptr, len);
}
