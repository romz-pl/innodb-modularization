#include <innodb/buffer/buf_page_alloc_descriptor.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_t.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/assert/assert.h>
#include <innodb/memory_check/memory_check.h>

/** Allocates a buf_page_t descriptor. This function must succeed. In case
 of failure we assert in this function.
 @return: the allocated descriptor. */
buf_page_t *buf_page_alloc_descriptor() {
  buf_page_t *bpage;

  bpage = (buf_page_t *)ut_zalloc_nokey(sizeof *bpage);
  ut_ad(bpage);
  UNIV_MEM_ALLOC(bpage, sizeof *bpage);

  return (bpage);
}

#endif
