#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/memory/mem_heap_t.h>

struct hash_table_t;


/** Gets the nth heap in a hash table.
@param[in]	table	hash table
@param[in]	i	index of the mutex
@return mem heap */
mem_heap_t *hash_get_nth_heap(hash_table_t *table, ulint i);


#endif
