#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to read the offset of the start of a data
 field in the record. The start of an SQL null field is the end offset of the
 previous non-null field, or 0, if none exists. If n is the number of the last
 field + 1, then the end offset of the last field is returned.
 @return offset of the start of the field */
ulint rec_get_field_start_offs(const rec_t *rec, /*!< in: record */
                               ulint n);          /*!< in: field index */
