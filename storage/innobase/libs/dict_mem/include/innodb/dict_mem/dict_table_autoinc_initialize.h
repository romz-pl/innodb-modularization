#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Unconditionally set the autoinc counter. */
void dict_table_autoinc_initialize(
    dict_table_t *table, /*!< in/out: table */
    ib_uint64_t value);  /*!< in: next value to assign to a row */
