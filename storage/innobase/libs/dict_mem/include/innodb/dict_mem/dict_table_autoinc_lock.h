#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Acquire the autoinc lock. */
void dict_table_autoinc_lock(dict_table_t *table); /*!< in/out: table */
