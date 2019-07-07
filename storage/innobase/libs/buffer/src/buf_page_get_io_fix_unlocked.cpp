#include <innodb/buffer/buf_page_get_io_fix_unlocked.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_t.h>

/** Gets the io_fix state of a buffer page. Does not assert that the
buf_page_get_mutex() mutex is held, to be used in the cases where it is safe
not to hold it.
@param[in]	bpage	pointer to the buffer page
@return page io_fix state */
buf_io_fix buf_page_get_io_fix_unlocked(const buf_page_t *bpage) {
  ut_ad(bpage != NULL);

  enum buf_io_fix io_fix = bpage->io_fix;

#ifdef UNIV_DEBUG
  switch (io_fix) {
    case BUF_IO_NONE:
    case BUF_IO_READ:
    case BUF_IO_WRITE:
    case BUF_IO_PIN:
      return (io_fix);
  }
  ut_error;
#else  /* UNIV_DEBUG */
  return (io_fix);
#endif /* UNIV_DEBUG */
}

#endif
