#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Read virtual column index from undo log if the undo log contains such
info, and verify the column is still indexed, and output its position
@param[in]	table		the table
@param[in]	ptr		undo log pointer
@param[in]	first_v_col	if this is the first virtual column, which
                                has the version marker
@param[in,out]	is_undo_log	his function is used to parse both undo log,
                                and online log for virtual columns. So
                                check to see if this is undo log
@param[out]	field_no	the column number
@return remaining part of undo log record after reading these values */
const byte *trx_undo_read_v_idx(const dict_table_t *table, const byte *ptr,
                                bool first_v_col, bool *is_undo_log,
                                ulint *field_no);
