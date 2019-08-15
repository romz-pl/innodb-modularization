#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Unlock the latch that has been locked by dict_table_stats_lock().
@param[in]	table		table whose stats to unlock
@param[in]	latch_mode	RW_S_LATCH or RW_X_LATCH */
void dict_table_stats_unlock(dict_table_t *table, ulint latch_mode);
