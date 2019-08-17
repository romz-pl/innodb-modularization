#include <innodb/mtr/mlog_catenate_ull_compressed.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/machine/data.h>

/** Catenates a compressed 64-bit integer to mlog. */
void mlog_catenate_ull_compressed(mtr_t *mtr,      /*!< in: mtr */
                                  ib_uint64_t val) /*!< in: value to write */
{
  byte *log_ptr;

  log_ptr = mlog_open(mtr, 15);

  /* If no logging is requested, we may return now */
  if (log_ptr == NULL) {
    return;
  }

  log_ptr += mach_u64_write_compressed(log_ptr, val);

  mlog_close(mtr, log_ptr);
}
