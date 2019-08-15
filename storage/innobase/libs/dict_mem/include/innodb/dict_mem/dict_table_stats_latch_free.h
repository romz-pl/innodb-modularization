#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

void dict_table_stats_latch_free(dict_table_t *table);
