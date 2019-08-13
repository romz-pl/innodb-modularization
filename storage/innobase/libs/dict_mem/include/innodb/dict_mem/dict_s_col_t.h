#pragma once

#include <innodb/univ/univ.h>

struct dict_col_t;

/** Data structure for a stored column in a table. */
struct dict_s_col_t {
  /** Stored column ptr */
  dict_col_t *m_col;
  /** array of base col ptr */
  dict_col_t **base_col;
  /** number of base columns */
  ulint num_base;
  /** column pos in table */
  ulint s_pos;
};
