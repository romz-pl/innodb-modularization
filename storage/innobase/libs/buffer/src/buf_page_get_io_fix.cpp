#include <innodb/buffer/buf_page_get_io_fix.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buffer/buf_page_get_mutex.h>
#include <innodb/buffer/buf_page_get_io_fix_unlocked.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Gets the io_fix state of a block.
 @return io_fix state */
buf_io_fix buf_page_get_io_fix(
    const buf_page_t *bpage) /*!< in: pointer to the control block */
{
  ut_ad(mutex_own(buf_page_get_mutex(bpage)));
  return buf_page_get_io_fix_unlocked(bpage);
}

#endif
