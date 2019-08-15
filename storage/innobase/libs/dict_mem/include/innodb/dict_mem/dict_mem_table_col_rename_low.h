#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

void dict_mem_table_col_rename_low(
    dict_table_t *table, /*!< in/out: table */
    unsigned i,          /*!< in: column offset corresponding to s */
    const char *to,      /*!< in: new column name */
    const char *s,       /*!< in: pointer to table->col_names */
    bool is_virtual);
