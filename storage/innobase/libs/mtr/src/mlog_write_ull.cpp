#include <innodb/mtr/mlog_write_ull.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/machine/data.h>
#include <innodb/page/page_offset.h>


/** Writes 8 bytes to a file page. Writes the corresponding log
 record to the mini-transaction log, only if mtr is not NULL */
void mlog_write_ull(byte *ptr,       /*!< in: pointer where to write */
                    ib_uint64_t val, /*!< in: value to write */
                    mtr_t *mtr)      /*!< in: mini-transaction handle */
{
  mach_write_to_8(ptr, val);

  if (mtr != 0) {
    byte *log_ptr = mlog_open(mtr, 11 + 2 + 9);

    /* If no logging is requested, we may return now */
    if (log_ptr != 0) {
      log_ptr =
          mlog_write_initial_log_record_fast(ptr, MLOG_8BYTES, log_ptr, mtr);

      mach_write_to_2(log_ptr, page_offset(ptr));
      log_ptr += 2;

      log_ptr += mach_u64_write_compressed(log_ptr, val);

      mlog_close(mtr, log_ptr);
    }
  }
}
