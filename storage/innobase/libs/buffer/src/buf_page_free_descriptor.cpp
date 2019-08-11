#include <innodb/buffer/buf_page_free_descriptor.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/allocator/ut_free.h>

/** Free a buf_page_t descriptor. */
void buf_page_free_descriptor(
    buf_page_t *bpage) /*!< in: bpage descriptor to free. */
{
  ut_free(bpage);
}

#endif
