#include <innodb/buf_pool/buf_pool_get.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/page/page_id_t.h>
#include <innodb/buf_pool/buf_pool_ptr.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Returns the buffer pool instance given a page id.
@param[in]	page_id	page id
@return buffer pool */
buf_pool_t *buf_pool_get(const page_id_t &page_id) {
  /* 2log of BUF_READ_AHEAD_AREA (64) */
  page_no_t ignored_page_no = page_id.page_no() >> 6;

  page_id_t id(page_id.space(), ignored_page_no);

  ulint i = id.fold() % srv_buf_pool_instances;

  return (&buf_pool_ptr[i]);
}

#endif
