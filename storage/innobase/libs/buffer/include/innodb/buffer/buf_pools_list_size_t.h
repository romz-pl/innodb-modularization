#pragma once

#include <innodb/univ/univ.h>

/** The occupied bytes of lists in all buffer pools */
struct buf_pools_list_size_t {
  ulint LRU_bytes;        /*!< LRU size in bytes */
  ulint unzip_LRU_bytes;  /*!< unzip_LRU size in bytes */
  ulint flush_list_bytes; /*!< flush_list size in bytes */
};
