#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Change the id of a table object in the dictionary cache. This is used in
 DISCARD TABLESPACE. */
void dict_table_change_id_in_cache(
    dict_table_t *table, /*!< in/out: table object already in cache */
    table_id_t new_id);  /*!< in: new id to set */
