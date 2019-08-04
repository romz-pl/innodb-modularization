#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct hash_table_t;

#ifndef UNIV_HOTBACKUP

/** Gets the heap for a fold value in a hash table.
@param[in]	table	hash table
@param[in]	fold	fold
@return mem heap */
mem_heap_t *hash_get_heap(hash_table_t *table, ulint fold);

#else

#define hash_get_heap(table, fold) ((table)->heap)

#endif
