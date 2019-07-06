#pragma once

#include <innodb/univ/univ.h>

/** Algorithm to remove the pages for a tablespace from the buffer pool.
See buf_LRU_flush_or_remove_pages(). */
enum buf_remove_t {
  BUF_REMOVE_ALL_NO_WRITE,   /*!< Remove all pages from the buffer
                             pool, don't write or sync to disk */
  BUF_REMOVE_FLUSH_NO_WRITE, /*!< Remove only, from the flush list,
                             don't write or sync to disk */
  BUF_REMOVE_FLUSH_WRITE     /*!< Flush dirty pages to disk only
                             don't remove from the buffer pool */
};
