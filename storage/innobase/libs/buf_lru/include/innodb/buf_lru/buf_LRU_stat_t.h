#pragma once

#include <innodb/univ/univ.h>

/** @brief Statistics for selecting the LRU list for eviction.

These statistics are not 'of' LRU but 'for' LRU.  We keep count of I/O
and page_zip_decompress() operations.  Based on the statistics we decide
if we want to evict from buf_pool->unzip_LRU or buf_pool->LRU. */
struct buf_LRU_stat_t {
  ulint io;    /**< Counter of buffer pool I/O operations. */
  ulint unzip; /**< Counter of page_zip_decompress operations. */
};
