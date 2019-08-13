#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_mem/dict_v_idx_list.h>

/** Data structure for a virtual column in a table */
struct dict_v_col_t {
  /** column structure */
  dict_col_t m_col;

  /** array of base column ptr */
  dict_col_t **base_col;

  /** number of base column */
  ulint num_base;

  /** column pos in table */
  ulint v_pos;

  /** Virtual index list, and column position in the index,
  the allocated memory is not from table->heap, nor it is
  tracked by dict_sys->size */
  dict_v_idx_list *v_indexes;
};
