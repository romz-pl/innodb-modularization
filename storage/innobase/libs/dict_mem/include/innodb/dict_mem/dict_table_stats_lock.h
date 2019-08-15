#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Lock the appropriate latch to protect a given table's statistics.
@param[in]	table		table whose stats to lock
@param[in]	latch_mode	RW_S_LATCH or RW_X_LATCH */
void dict_table_stats_lock(dict_table_t *table, ulint latch_mode);
