#include <innodb/mtr/mlog_write_string.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mlog_log_string.h>

/** Writes a string to a file page buffered in the buffer pool. Writes the
 corresponding log record to the mini-transaction log. */
void mlog_write_string(byte *ptr,       /*!< in: pointer where to write */
                       const byte *str, /*!< in: string to write */
                       ulint len,       /*!< in: string length */
                       mtr_t *mtr)      /*!< in: mini-transaction handle */
{
  ut_ad(ptr && mtr);
  ut_a(len < UNIV_PAGE_SIZE);

  memcpy(ptr, str, len);

  mlog_log_string(ptr, len, mtr);
}


#endif /* !UNIV_HOTBACKUP */
