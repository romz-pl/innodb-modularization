#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dict_table_t;
struct dtuple_t;

/** Read virtual column value from undo log
@param[in]	table		the table
@param[in]	ptr		undo log pointer
@param[in,out]	row		the dtuple to fill
@param[in]	in_purge        called by purge thread
@param[in]	online		true if this is from online DDL log
@param[in]	col_map		online rebuild column map
@param[in,out]	heap		memory heap to keep value when necessary */
void trx_undo_read_v_cols(const dict_table_t *table, const byte *ptr,
                          const dtuple_t *row, bool in_purge, bool online,
                          const ulint *col_map, mem_heap_t *heap);
