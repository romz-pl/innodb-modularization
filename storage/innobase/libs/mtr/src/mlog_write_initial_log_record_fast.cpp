#include <innodb/mtr/mlog_write_initial_log_record_fast.h>

#include <innodb/align/ut_align_down.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/mtr/mlog_write_initial_log_record_low.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/trx_sys/flags.h>

/** Set to TRUE when the doublewrite buffer is being created */
extern ibool buf_dblwr_being_created;

#ifndef UNIV_HOTBACKUP
/** Writes the initial part of a log record (3..11 bytes).
 If the implementation of this function is changed, all
 size parameters to mlog_open() should be adjusted accordingly!
 @return new value of log_ptr */
byte *mlog_write_initial_log_record_fast(
    const byte *ptr, /*!< in: pointer to (inside) a buffer
                     frame holding the file page where
                     modification is made */
    mlog_id_t type,  /*!< in: log item type: MLOG_1BYTE, ... */
    byte *log_ptr,   /*!< in: pointer to mtr log which has
                     been opened */
    mtr_t *mtr)      /*!< in/out: mtr */
{
  const byte *page;
  space_id_t space;
  page_no_t offset;

  ut_ad(log_ptr);
  ut_d(mtr->memo_modify_page(ptr));

  page = (const byte *)ut_align_down(ptr, UNIV_PAGE_SIZE);
  space = mach_read_from_4(page + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);
  offset = mach_read_from_4(page + FIL_PAGE_OFFSET);

  /* check whether the page is in the doublewrite buffer;
  the doublewrite buffer is located in pages
  FSP_EXTENT_SIZE, ..., 3 * FSP_EXTENT_SIZE - 1 in the
  system tablespace */

  if (space == TRX_SYS_SPACE && offset >= FSP_EXTENT_SIZE &&
      offset < 3 * FSP_EXTENT_SIZE) {
    if (buf_dblwr_being_created) {
      /* Do nothing: we only come to this branch in an
      InnoDB database creation. We do not redo log
      anything for the doublewrite buffer pages. */
      return (log_ptr);
    } else {
#ifdef UNIV_NO_ERR_MSGS
      ib::error()
#else
      ib::error(ER_IB_MSG_1269)
#endif /* !UNIV_NO_ERR_MSGS */
          << "Trying to redo log a record of type " << type << "  on page "
          << page_id_t(space, offset)
          << "in the"
             " doublewrite buffer, continuing anyway."
             " Please post a bug report to"
             " bugs.mysql.com.";
      ut_ad(0);
    }
  }

  return (mlog_write_initial_log_record_low(type, space, offset, log_ptr, mtr));
}
#endif /* !UNIV_HOTBACKUP */
