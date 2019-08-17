#include <innodb/mtr/mlog_write_ulint.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/machine/data.h>
#include <innodb/error/ut_error.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/page/page_offset.h>


/** Writes 1, 2 or 4 bytes to a file page. Writes the corresponding log
 record to the mini-transaction log if mtr is not NULL. */
void mlog_write_ulint(
    byte *ptr,      /*!< in: pointer where to write */
    ulint val,      /*!< in: value to write */
    mlog_id_t type, /*!< in: MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES */
    mtr_t *mtr)     /*!< in: mini-transaction handle */
{
  switch (type) {
    case MLOG_1BYTE:
      mach_write_to_1(ptr, val);
      break;
    case MLOG_2BYTES:
      mach_write_to_2(ptr, val);
      break;
    case MLOG_4BYTES:
      mach_write_to_4(ptr, val);
      break;
    default:
      ut_error;
  }

  if (mtr != 0) {
    byte *log_ptr = mlog_open(mtr, 11 + 2 + 5);

    /* If no logging is requested, we may return now */

    if (log_ptr != 0) {
      log_ptr = mlog_write_initial_log_record_fast(ptr, type, log_ptr, mtr);

      mach_write_to_2(log_ptr, page_offset(ptr));
      log_ptr += 2;

      log_ptr += mach_write_compressed(log_ptr, val);

      mlog_close(mtr, log_ptr);
    }
  }
}
