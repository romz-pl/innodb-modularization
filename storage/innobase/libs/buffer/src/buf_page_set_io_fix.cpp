#include <innodb/buffer/buf_page_set_io_fix.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_t.h>
#include <innodb/buffer/buf_page_get_mutex.h>
#include <innodb/buffer/buf_page_get_io_fix.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Sets the io_fix state of a block. */
void buf_page_set_io_fix(buf_page_t *bpage,      /*!< in/out: control block */
                         buf_io_fix io_fix) /*!< in: io_fix state */
{
  ut_ad(mutex_own(buf_page_get_mutex(bpage)));

  bpage->io_fix = io_fix;
  ut_ad(buf_page_get_io_fix(bpage) == io_fix);
}

#endif
