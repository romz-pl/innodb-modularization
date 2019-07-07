#include <innodb/buffer/buf_page_can_relocate.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_t.h>
#include <innodb/buffer/buf_page_get_mutex.h>
#include <innodb/buffer/buf_page_in_file.h>
#include <innodb/buffer/buf_page_get_io_fix.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Determine if a buffer block can be relocated in memory.  The block
 can be dirty, but it must not be I/O-fixed or bufferfixed. */
ibool buf_page_can_relocate(
    const buf_page_t *bpage) /*!< control block being relocated */
{
  ut_ad(mutex_own(buf_page_get_mutex(bpage)));
  ut_ad(buf_page_in_file(bpage));
  ut_ad(bpage->in_LRU_list);

  return (buf_page_get_io_fix(bpage) == BUF_IO_NONE &&
          bpage->buf_fix_count == 0);
}

#endif
