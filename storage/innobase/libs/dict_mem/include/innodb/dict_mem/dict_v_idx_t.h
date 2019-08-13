#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

/** Index information put in a list of virtual column structure. Index
id and virtual column position in the index will be logged.
There can be multiple entries for a given index, with a different position. */
struct dict_v_idx_t {
  /** active index on the column */
  dict_index_t *index;

  /** position in this index */
  ulint nth_field;
};
