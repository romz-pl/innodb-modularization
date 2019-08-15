#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Reads the next autoinc value (== autoinc counter value), 0 if not yet
 initialized.
 @return value for a new row, or 0 */
ib_uint64_t dict_table_autoinc_read(const dict_table_t *table) /*!< in: table */
    MY_ATTRIBUTE((warn_unused_result));
