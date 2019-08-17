#include <innodb/buf_lru/buf_LRU_stat_sum.h>

/** Running sum of past values of buf_LRU_stat_cur.
Updated by buf_LRU_stat_update().  Not Protected by any mutex. */
buf_LRU_stat_t buf_LRU_stat_sum;
