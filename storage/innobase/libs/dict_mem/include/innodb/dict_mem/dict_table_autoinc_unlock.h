#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Release the autoinc lock. */
void dict_table_autoinc_unlock(dict_table_t *table); /*!< in/out: table */
