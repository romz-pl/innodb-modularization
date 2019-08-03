#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Sets an old-style record field to SQL null.
 The physical size of the field is not changed. */
void rec_set_nth_field_sql_null(rec_t *rec, /*!< in: record */
                                ulint n);    /*!< in: index of the field */
