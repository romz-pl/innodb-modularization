#include <innodb/buffer/buf_page_set_flush_type.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_t.h>
#include <innodb/buffer/buf_page_get_flush_type.h>

/** Set the flush type of a page. */
void buf_page_set_flush_type(buf_page_t *bpage,      /*!< in: buffer page */
                             buf_flush_t flush_type) /*!< in: flush type */
{
  bpage->flush_type = flush_type;
  ut_ad(buf_page_get_flush_type(bpage) == flush_type);
}

#endif
