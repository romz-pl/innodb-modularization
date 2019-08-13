#pragma once

#include <innodb/univ/univ.h>

struct dict_v_col_t;

/** Data structure for newly added virtual column in a table */
struct dict_add_v_col_t {
  /** number of new virtual column */
  ulint n_v_col;

  /** column structures */
  const dict_v_col_t *v_col;

  /** new col names */
  const char **v_col_name;
};
