#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;
struct dict_table_t;

/** Copies types of columns contained in table to tuple and sets all
 fields of the tuple to the SQL NULL value.  This function should
 be called right after dtuple_create(). */
void dict_table_copy_types(dtuple_t *tuple, /*!< in/out: data tuple */
                           const dict_table_t *table); /*!< in: table */
