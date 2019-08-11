#include <innodb/buf_page/buf_page_get_newest_modification.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_block/BPageMutex.h>
#include <innodb/buf_page/buf_page_get_mutex.h>
#include <innodb/buf_page/buf_page_in_file.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Gets the youngest modification log sequence number for a frame.
 Returns zero if not file page or no modification occurred yet.
 @return newest modification to page */
lsn_t buf_page_get_newest_modification(
    const buf_page_t *bpage) /*!< in: block containing the
                             page frame */
{
  lsn_t lsn;
  BPageMutex *block_mutex = buf_page_get_mutex(bpage);

  mutex_enter(block_mutex);

  if (buf_page_in_file(bpage)) {
    lsn = bpage->newest_modification;
  } else {
    lsn = 0;
  }

  mutex_exit(block_mutex);

  return (lsn);
}

#endif
