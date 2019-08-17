#include <innodb/mtr/mlog_catenate_ulint_compressed.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/machine/data.h>

/** Catenates a compressed ulint to mlog. */
void mlog_catenate_ulint_compressed(mtr_t *mtr, /*!< in: mtr */
                                    ulint val)  /*!< in: value to write */
{
  byte *log_ptr;

  log_ptr = mlog_open(mtr, 10);

  /* If no logging is requested, we may return now */
  if (log_ptr == NULL) {
    return;
  }

  log_ptr += mach_write_compressed(log_ptr, val);

  mlog_close(mtr, log_ptr);
}
