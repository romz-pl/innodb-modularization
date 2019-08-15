#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;
struct dict_foreign_t;

dict_foreign_t *dict_foreign_find(
    dict_table_t *table,     /*!< in: table object */
    dict_foreign_t *foreign); /*!< in: foreign constraint */
