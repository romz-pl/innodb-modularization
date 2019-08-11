#include <innodb/buf_pool/buf_pool_get_n_pages.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_pool/buf_pool_get_curr_size.h>

/** Gets the current size of buffer buf_pool in pages.
 @return size in pages*/
ulint buf_pool_get_n_pages() {
  return (buf_pool_get_curr_size() / UNIV_PAGE_SIZE);
}

#endif
