#include <innodb/mtr/mlog_open.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mtr_get_log_mode.h>


/** Opens a buffer to mlog. It must be closed with mlog_close.
@param[in,out]	mtr	mtr
@param[in]	size	buffer size in bytes; MUST be smaller than
                        DYN_ARRAY_DATA_SIZE!
@return buffer, NULL if log mode MTR_LOG_NONE or MTR_LOG_NO_REDO */
byte *mlog_open(mtr_t *mtr, ulint size) {
  mtr->set_modified();

  if (mtr_get_log_mode(mtr) == MTR_LOG_NONE ||
      mtr_get_log_mode(mtr) == MTR_LOG_NO_REDO) {
    return (NULL);
  }

  return (mtr->get_log()->open(size));
}
