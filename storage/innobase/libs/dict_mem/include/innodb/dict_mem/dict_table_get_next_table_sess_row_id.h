#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Get table session row-id and increment the row-id counter for next use.
@param[in,out]	table	table handler
@return next table session row-id. */
UNIV_INLINE
row_id_t dict_table_get_next_table_sess_row_id(dict_table_t *table) {
  return (++table->sess_row_id);
}
