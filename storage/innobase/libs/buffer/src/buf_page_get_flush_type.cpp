#include <innodb/buffer/buf_page_get_flush_type.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>

/** Get the flush type of a page.
 @return flush type */
buf_flush_t buf_page_get_flush_type(
    const buf_page_t *bpage) /*!< in: buffer page */
{
  buf_flush_t flush_type = (buf_flush_t)bpage->flush_type;

#ifdef UNIV_DEBUG
  switch (flush_type) {
    case BUF_FLUSH_LRU:
    case BUF_FLUSH_LIST:
    case BUF_FLUSH_SINGLE_PAGE:
      return (flush_type);
    case BUF_FLUSH_N_TYPES:
      ut_error;
  }
  ut_error;
#else  /* UNIV_DEBUG */
  return (flush_type);
#endif /* UNIV_DEBUG */
}

#endif
