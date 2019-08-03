#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** This is used to modify the value of an already existing field in a record.
 The previous value must have exactly the same size as the new value. If len
 is UNIV_SQL_NULL then the field is treated as an SQL null.
 For records in ROW_FORMAT=COMPACT (new-style records), len must not be
 UNIV_SQL_NULL unless the field already is SQL null. */
void rec_set_nth_field(
    rec_t *rec,           /*!< in: record */
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n,              /*!< in: index number of the field */
    const void *data,     /*!< in: pointer to the data
                          if not SQL null */
    ulint len);            /*!< in: length of the data or UNIV_SQL_NULL */
