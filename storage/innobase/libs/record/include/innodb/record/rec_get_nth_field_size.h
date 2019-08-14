#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Gets the physical size of an old-style field.
 Also an SQL null may have a field of size > 0,
 if the data type is of a fixed size.
 @return field size in bytes */
ulint rec_get_nth_field_size(const rec_t *rec, /*!< in: record */
                             ulint n)          /*!< in: index of the field */
    MY_ATTRIBUTE((warn_unused_result));
