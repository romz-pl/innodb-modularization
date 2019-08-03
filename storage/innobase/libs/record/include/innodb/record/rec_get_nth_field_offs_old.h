#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to get the offset to the nth
 data field in an old-style record.
 @return offset to the field */
ulint rec_get_nth_field_offs_old(
    const rec_t *rec, /*!< in: record */
    ulint n,          /*!< in: index of the field */
    ulint *len);      /*!< out: length of the field; UNIV_SQL_NULL
                      if SQL null */
