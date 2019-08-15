#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_vcol_set.h>

struct dict_index_t;

void dict_mem_fill_vcol_has_index(const dict_index_t *index,
                                         dict_vcol_set **v_cols);
