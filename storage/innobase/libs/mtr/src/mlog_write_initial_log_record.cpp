#include <innodb/mtr/mlog_write_initial_log_record.h>

#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/assert/assert.h>


#ifndef UNIV_HOTBACKUP
/** Writes the initial part of a log record consisting of one-byte item
 type and four-byte space and page numbers. Also pushes info
 to the mtr memo that a buffer page has been modified. */
void mlog_write_initial_log_record(
    const byte *ptr, /*!< in: pointer to (inside) a buffer
                     frame holding the file page where
                     modification is made */
    mlog_id_t type,  /*!< in: log item type: MLOG_1BYTE, ... */
    mtr_t *mtr)      /*!< in: mini-transaction handle */
{
  byte *log_ptr;

  ut_ad(type <= MLOG_BIGGEST_TYPE);
  ut_ad(type > MLOG_8BYTES);

  log_ptr = mlog_open(mtr, 11);

  /* If no logging is requested, we may return now */
  if (log_ptr == NULL) {
    return;
  }

  log_ptr = mlog_write_initial_log_record_fast(ptr, type, log_ptr, mtr);

  mlog_close(mtr, log_ptr);
}
#endif /* !UNIV_HOTBACKUP */
