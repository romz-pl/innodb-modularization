#include <innodb/buf_lru/buf_LRU_stat_cur.h>


/** Current operation counters.  Not protected by any mutex.  Cleared
by buf_LRU_stat_update(). */
buf_LRU_stat_t buf_LRU_stat_cur;
