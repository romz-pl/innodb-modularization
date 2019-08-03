#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Sets the value of the ith field SQL null bit of an old-style record. */
void rec_set_nth_field_null_bit(rec_t *rec, /*!< in: record */
                                ulint i,    /*!< in: ith field */
                                ibool val); /*!< in: value to set */
