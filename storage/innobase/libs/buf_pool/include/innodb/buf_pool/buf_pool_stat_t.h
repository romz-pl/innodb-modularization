#pragma once

#include <innodb/univ/univ.h>

/** @brief The buffer pool statistics structure. */
struct buf_pool_stat_t {
  ulint n_page_gets;            /*!< number of page gets performed;
                                also successful searches through
                                the adaptive hash index are
                                counted as page gets; this field
                                is NOT protected by the buffer
                                pool mutex */
  ulint n_pages_read;           /*!< number of read operations. Accessed
                                atomically. */
  ulint n_pages_written;        /*!< number of write operations. Accessed
                                atomically. */
  ulint n_pages_created;        /*!< number of pages created
                                in the pool with no read. Accessed
                                atomically. */
  ulint n_ra_pages_read_rnd;    /*!< number of pages read in
                            as part of random read ahead. Not protected. */
  ulint n_ra_pages_read;        /*!< number of pages read in
                                as part of read ahead. Not protected. */
  ulint n_ra_pages_evicted;     /*!< number of read ahead
                             pages that are evicted without
                             being accessed. Protected by LRU_list_mutex. */
  ulint n_pages_made_young;     /*!< number of pages made young, in
                            calls to buf_LRU_make_block_young(). Protected
                            by LRU_list_mutex. */
  ulint n_pages_not_made_young; /*!< number of pages not made
                        young because the first access
                        was not long enough ago, in
                        buf_page_peek_if_too_old(). Not protected. */
  ulint LRU_bytes;              /*!< LRU size in bytes. Protected by
                                LRU_list_mutex. */
  ulint flush_list_bytes;       /*!< flush_list size in bytes.
                               Protected by flush_list_mutex */
};
