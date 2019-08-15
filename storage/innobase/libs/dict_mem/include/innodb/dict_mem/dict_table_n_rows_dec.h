#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Decrement the number of rows in the table by one.
 Notice that this operation is not protected by any latch, the number is
 approximate. */
UNIV_INLINE
void dict_table_n_rows_dec(dict_table_t *table) /*!< in/out: table */
{
  if (table->stat_initialized) {
    ib_uint64_t n_rows = table->stat_n_rows;
    if (n_rows > 0) {
      table->stat_n_rows = n_rows - 1;
    }
  }
}
