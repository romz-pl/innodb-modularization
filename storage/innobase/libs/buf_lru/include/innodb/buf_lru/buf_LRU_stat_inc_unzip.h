#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_lru/buf_LRU_stat_t.h>

/** Current operation counters.  Not protected by any mutex.
Cleared by buf_LRU_stat_update(). */
extern buf_LRU_stat_t buf_LRU_stat_cur;

/** Increments the page_zip_decompress() counter in buf_LRU_stat_cur. */
#define buf_LRU_stat_inc_unzip() buf_LRU_stat_cur.unzip++
