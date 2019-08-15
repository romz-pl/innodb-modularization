#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;
struct dict_index_t;
class index_id_t;

const dict_index_t *dict_table_find_index_on_id(
    const dict_table_t *table, const index_id_t &id);
