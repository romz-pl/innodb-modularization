#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

#ifdef UNIV_DEBUG

/** Gets the nth virtual column of a table.
@param[in]	table	table
@param[in]	pos	position of virtual column
@return pointer to virtual column object */
UNIV_INLINE
dict_v_col_t *dict_table_get_nth_v_col(const dict_table_t *table, ulint pos);

#else /* UNIV_DEBUG */

/* Get nth virtual columns */
#define dict_table_get_nth_v_col(table, pos) ((table)->v_cols + (pos))

#endif /* UNIV_DEBUG */
