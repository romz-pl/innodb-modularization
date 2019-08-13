#pragma once

#include <innodb/univ/univ.h>

struct dict_col_t;

/** Data structure for default value of a column in a table */
struct dict_col_default_t {
  /** Pointer to the column itself */
  dict_col_t *col;
  /** Default value in bytes */
  byte *value;
  /** Length of default value */
  size_t len;
};
