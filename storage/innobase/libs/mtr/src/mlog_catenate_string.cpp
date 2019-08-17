#include <innodb/mtr/mlog_catenate_string.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mtr_get_log_mode.h>

/** Catenates n bytes to the mtr log. */
void mlog_catenate_string(mtr_t *mtr,      /*!< in: mtr */
                          const byte *str, /*!< in: string to write */
                          ulint len)       /*!< in: string length */
{
  if (mtr_get_log_mode(mtr) == MTR_LOG_NONE) {
    return;
  }

  mtr->get_log()->push(str, ib_uint32_t(len));
}
