#include <innodb/buffer/buf_page_unset_sticky.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buffer/buf_page_get_mutex.h>
#include <innodb/buffer/buf_page_get_io_fix.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Removes stickiness of a block. */
void buf_page_unset_sticky(buf_page_t *bpage) /*!< in/out: control block */
{
  ut_ad(mutex_own(buf_page_get_mutex(bpage)));
  ut_ad(buf_page_get_io_fix(bpage) == BUF_IO_PIN);

  bpage->io_fix = BUF_IO_NONE;
}

#endif
