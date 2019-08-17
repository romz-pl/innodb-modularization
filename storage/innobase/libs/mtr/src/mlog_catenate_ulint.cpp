#include <innodb/mtr/mlog_catenate_ulint.h>

#include <innodb/machine/data.h>
#include <innodb/error/ut_error.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mtr_get_log_mode.h>


/** Catenates 1 - 4 bytes to the mtr log. The value is not compressed. */
void mlog_catenate_ulint(mtr_buf_t *mtr_buf, /*!< in/out: buffer to write */
                         ulint val,          /*!< in: value to write */
                         mlog_id_t type)     /*!< in: type of value to write */
{
  ut_ad(MLOG_1BYTE == 1);
  ut_ad(MLOG_2BYTES == 2);
  ut_ad(MLOG_4BYTES == 4);
  ut_ad(MLOG_8BYTES == 8);

  byte *ptr = mtr_buf->push<byte *>(type);

  switch (type) {
    case MLOG_4BYTES:
      mach_write_to_4(ptr, val);
      break;
    case MLOG_2BYTES:
      mach_write_to_2(ptr, val);
      break;
    case MLOG_1BYTE:
      mach_write_to_1(ptr, val);
      break;
    default:
      ut_error;
  }
}


/** Catenates 1 - 4 bytes to the mtr log. The value is not compressed. */
void mlog_catenate_ulint(
    mtr_t *mtr,     /*!< in/out: mtr */
    ulint val,      /*!< in: value to write */
    mlog_id_t type) /*!< in: MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES */
{
  if (mtr_get_log_mode(mtr) == MTR_LOG_NONE ||
      mtr_get_log_mode(mtr) == MTR_LOG_NO_REDO) {
    return;
  }

  mlog_catenate_ulint(mtr->get_log(), val, type);
}
