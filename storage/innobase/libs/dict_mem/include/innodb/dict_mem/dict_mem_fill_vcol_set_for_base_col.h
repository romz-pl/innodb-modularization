#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_vcol_set.h>

struct dict_table_t;

void dict_mem_fill_vcol_set_for_base_col(const char *col_name,
                                                const dict_table_t *table,
                                                dict_vcol_set **v_cols);
