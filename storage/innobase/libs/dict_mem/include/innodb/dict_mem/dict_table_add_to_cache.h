#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dict_table_t;

/** Adds a table object to the dictionary cache.
@param[in]	table		table
@param[in]	can_be_evicted	true if can be evicted
@param[in]	heap		temporary heap
*/
void dict_table_add_to_cache(dict_table_t *table, ibool can_be_evicted,
                             mem_heap_t *heap);
