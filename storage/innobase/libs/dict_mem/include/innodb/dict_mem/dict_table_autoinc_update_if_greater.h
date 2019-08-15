#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Updates the autoinc counter if the value supplied is greater than the
 current value. */
void dict_table_autoinc_update_if_greater(

    dict_table_t *table, /*!< in/out: table */
    ib_uint64_t value);  /*!< in: value which was assigned to a row */
