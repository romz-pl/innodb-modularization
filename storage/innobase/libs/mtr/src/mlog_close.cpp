#include <innodb/mtr/mlog_close.h>

#include <innodb/mtr/mtr_t.h>
#include <innodb/mtr/mtr_get_log_mode.h>

/** Closes a buffer opened to mlog. */
void mlog_close(mtr_t *mtr, /*!< in: mtr */
                byte *ptr)  /*!< in: buffer space from ptr up was not used */
{
  ut_ad(mtr_get_log_mode(mtr) != MTR_LOG_NONE);
  ut_ad(mtr_get_log_mode(mtr) != MTR_LOG_NO_REDO);

  mtr->get_log()->close(ptr);
}
