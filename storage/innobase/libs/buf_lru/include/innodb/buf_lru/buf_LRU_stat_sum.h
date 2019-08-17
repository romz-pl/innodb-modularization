#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_lru/buf_LRU_stat_t.h>

/** Running sum of past values of buf_LRU_stat_cur.
Updated by buf_LRU_stat_update(). Accesses protected by memory barriers. */
extern buf_LRU_stat_t buf_LRU_stat_sum;
