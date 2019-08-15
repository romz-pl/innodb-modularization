#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Create a dict_table_t's stats latch or delay for lazy creation.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	table	table whose stats latch to create
@param[in]	enabled	if false then the latch is disabled
and dict_table_stats_lock()/unlock() become noop on this table. */
void dict_table_stats_latch_create(dict_table_t *table, bool enabled);
