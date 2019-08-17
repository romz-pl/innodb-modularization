#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

void dict_mem_table_free(dict_table_t *table);
