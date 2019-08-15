#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Renames a column of a table in the data dictionary cache. */
void dict_mem_table_col_rename(dict_table_t *table, /*!< in/out: table */
                               ulint nth_col,       /*!< in: column index */
                               const char *from,    /*!< in: old column name */
                               const char *to,      /*!< in: new column name */
                               bool is_virtual);
/*!< in: if this is a virtual column */
